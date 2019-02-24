/* Copyright (c) 2009 Alexander Schmidt. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Alexander Schmidt."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact wrk@hpi.uni-potsdam.de.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL ALEXANDER SCHMIDT BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "makesystable.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

typedef std::vector<Service> ServiceList;

void ShowUsage(const char *ProgramName)
/*++

Routine Description:

    This function simply prints a usage string to the standard output.

Parameters:

    ProgramName - The name of the executable.

Return Value:

    None.

--*/
{
    cout << "Usage: " << ProgramName << " [Options]" << endl << endl;
    cout << "Options:  " << endl;
    cout << "  /machine:(x86|amd64)" << endl << "    Target architecture for the system call." << endl;
    cout << "  /dir:<Directory>" << endl << "    Directory where to create the system service table." << endl;
    cout << "    Note: This directory must also contain the 'services.tab' file." << endl;
    cout << "  /?" << endl << "    Shows this help message." << endl << endl;
}

bool GetCd(string &Cd)
/*++

Routine Description:

    This function determines the current working directory.

Parameters:

    Cd - A reference to a string object that will receive the
        path of the current working directory.

Return Value:

    The function returns true, if the current working directory path
    has been stored in the Cd parameter. False, otherwise.

--*/
{
    DWORD BufferLength = 0;
    PCHAR Buffer = NULL;
    Cd = "";

    BufferLength = GetCurrentDirectoryA(BufferLength, Buffer);
    if (BufferLength == 0)
    {
        cout << "Error getting current directory: " << GetLastError() << endl;
        return false;
    }

    Buffer = (PCHAR) malloc(BufferLength * sizeof(CHAR));
    if (!Buffer)
    {
        cout << "Not enough memory to get working directory." << endl;
        return false;
    }

    BufferLength = GetCurrentDirectoryA(BufferLength, Buffer);
    if (BufferLength == 0)
    {
        cout << "Error getting current directory: " << GetLastError() << endl;
        return false;
    }

    Cd.assign(Buffer, BufferLength);

    free(Buffer);
    return true;
}

void StripWhiteSpaces(string &line)
/*++

Routine Description:

    This function removes any leading white spaces from a string.

Parameters:

    line - The string where leading white spaces should be removed from.
        This is a reference parameter.

Return Value:

   None.

--*/
{
    string::size_type i;

    for (i = 0; i < line.length(); i++)
    {
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r')
        {
            break;
        }
    }

    line = line.substr(i);
}

bool ReadSystemServices(ifstream &serviceFile, ServiceList &services)
/*++

Routine Description:

    This function parses the services.tab file of the Windows Research Kernel.
    The file is respresented by the serviceFile parameter. It generates
    a list of services that are provided by the current kernel. Each entry
    in the "services" is of type Service and contains the name, the number of
    arguments (as int), and, for convenience the number of arguments as string.

Parameters:

    serviceFile - Input file stream to the services.tab file of the Windows
        Research Kernel. The file may have a different name. However, this
        function expects the same format within the file. 

        The format is as follows: Comments start with a semi-colon (;) and the
        rest of such a line is ignored by this function. For each system service
        there must be a seperate line in the following format:

        <ServiceName>, <Number of Arguments>

    services - A vector object where all the services are stored in.

Return value:

    True, if the file could successfully be parsed. False, otherwise.

--*/
{
    string line;
    stringstream converter;
    int nArgs;

    while (!serviceFile.eof())
    {
        std::getline(serviceFile, line);
        StripWhiteSpaces(line);
        if (line.empty())
            continue;
        if (line[0] == ';')
            continue;

        // cout << line.c_str() << "\n";
        string::size_type delimiter = line.find(",");
        if (delimiter == string::npos)
        {
            cerr << "Invalid format found for service declaration: " << line.c_str() << endl;
            continue;
        }

        string service = line.substr(0, delimiter);
        string argCount = line.substr(delimiter + 1);
        StripWhiteSpaces(argCount);

        converter.clear();
        converter << argCount.c_str();
        converter >> nArgs;

        Service *NewService = (Service *) malloc (sizeof(Service));
        if (!NewService)
        {
            cerr << "Not enough memory to create slot for new service" << endl;
            return false;
        }

        NewService->Name = new string(service);
        NewService->sArguments = new string(argCount);
        NewService->nArguments = nArgs;

        services.push_back(*NewService);

        //free(NewService);

    }

    return true;
}

int ReadInRegisterArguments(ifstream &TableStb)
/*++

Routin Description:

    This function reads exactly one line of a given file stream.
    It assumes this line contains solely an integer defining
    the number of arguments that are passed in registers when
    issuing a system service call. This is usually the first line
    of the table.stb file of the WRK (in base\ntos\ke\<arch>).

Parameters:

    TableStb - Input file stream to an open file that contains
        the number of in-register arguments as first line.

Return Value:

    The number of in-register arguments if an appropriate file
    was provided. The behaviour is however undefined if an arbitrary file
    at an arbitrary position was provided.

    Note: The file stream is modified when invoking this function.

--*/
{
    // The first line of the table.stb file contains
    // the number of in-register arguments that
    // is in the current target architecture.

    stringstream converter;
    string line;
    int InRegisterArguments;

    getline(TableStb, line);
    converter << line.c_str();
    converter >> InRegisterArguments;

    return InRegisterArguments;
}

bool DuplicateStubFile(ifstream &Input, ofstream &Output)
/*++

Routine Description:

    This function simply copies an input file stream to an output 
    file stream.

Parameters:

    Input - The input file stream. The file must already be open.

    Output - The output file stream. The file stream must have 
        been setup already.

Return Value:

    True, if the file was copied successfully. False otherwise.

--*/
{
    string line;

    while (!Input.eof())
    {
        getline(Input, line);
        Output << line.c_str() << endl;
    }

    return true;
}

int main(int argc, char* argv[])
{
    std::string CurrentParam;
    string NextParam;

    string cd = "";
    string machine = "x86";

    //
    // Fetch optional arguments.
    // The argument syntax is as follows:
    // [-|/]<argname>:<argvalue>
    // 
    // The only exception is the /? option.

    for (int i = 1; i < argc; i++)
    {
        CurrentParam = argv[i];
        if (CurrentParam[0] != '-' && CurrentParam[0] != '/')
        {
            cerr << "Unknown option: " << CurrentParam.c_str() << ". Abbort." << endl;
            return -1;
        }

        //
        // Find the first colon (:). We consider this as delimiter to delimit
        // the argument name from the argument value.
        //
        string::size_type delimiter = CurrentParam.find(":");
        if (delimiter == string::npos)
        {
            // 
            // No colon found. Check, if help menu was specified ...
            //
            if (CurrentParam.substr(1).compare("?") == 0)
            {
                ShowUsage(argv[0]);
                return 0;
            }
            else
            {
                cerr << "Invalid argument: " << CurrentParam.c_str() << ". Abbort." << endl;
                return -1;
            }
        }

        //
        // Split arg[i] in argument name and argument value.
        //

        string ParamName = CurrentParam.substr(1,delimiter - 1);
        string ParamValue = CurrentParam.substr(delimiter + 1, CurrentParam.length() - delimiter - 1);
       
        //
        // Check, which option was specified:
        //
        // Directory option.
        if (ParamName.compare("dir") == 0)
        {
            // 
            // We do not validate the given path here. Just store it for the 
            // moment.
            //

            cd = ParamValue;
        }
        // Machine option
        else if (ParamName.compare("machine") == 0)
        {
            //
            // If machine was specified, the only possible two values are x86 and amd64.
            //
            if ((ParamValue.compare("x86") == 0) || (ParamValue.compare("amd64") == 0))
            {
                machine = ParamValue;
            }
            else 
            {
                cerr << "Unknown machine option: " << ParamValue.c_str() << ". Abort." << endl;
                return -1;
            }
        }
        // unknown option.
        else
        {
            cerr << "Unknown option: " << ParamName.c_str() << ". Abort." << endl;
            return -1;
        }
    }

    // 
    // Check, if a directory was specified as command line option.
    //
    
    if (cd.compare("") == 0)
    {
        //
        // No directory specified, just take the current working directory
        // as default.
        //

        if (!GetCd(cd))
            return -1;
    }

    // 
    // For convenience, make sure the current directory end with a '\'.
    // This allows for easy path concatenation.
    //

    if (cd[cd.length() - 1] != '\\')
    {
        cd.append("\\");
    }



    string FileName = cd + "services.tab";
    
    // List of available services:
    ServiceList serviceList;

    // Read system services from services.tab
    ifstream services(FileName.c_str());
    if (services.fail())
    {
        cerr << "Unable to open file: \"" << FileName.c_str() << "\"" << endl;
        cerr << "Check directory settings." << endl;
        return -1;
    }

    // 
    // Parse the list of system services. This list is usually maintained
    // in the services.tab file in the base\ntos\ke\<architecture> 
    // directory.
    //

    ReadSystemServices(services, serviceList);
    services.close();

    // Open stub file for creating services.asm
    FileName = cd + "services.stb";
    ifstream SysstubsStb(FileName.c_str());
    if (SysstubsStb.fail())
    {
        cerr << "Unable to open file: \"" << FileName.c_str() << "\"" << endl;
        cerr << "Check directory settings." << endl;
        return -1;
    }
    
    // Open stub file for the actual service table
    FileName = cd + "table.stb";
    ifstream SystableStb(FileName.c_str());
    if (SystableStb.fail())
    {
        cerr << "Unable to open file: \"" << FileName.c_str() << "\"" << endl;
        cerr << "Check directory settings." << endl;
        return -1;
    }

    FileName = cd + "sysstubs.asm";
    ofstream SysstubsAsm(FileName.c_str());
    if (SysstubsAsm.fail())
    {
        cerr << "Unable to open file: \"" << FileName.c_str() << "\"" << endl;
        cerr << "Check directory settings." << endl;
        return -1;
    }

    FileName = cd + "systable.asm";
    ofstream SystableAsm(FileName.c_str());
    if (SystableAsm.fail())
    {
        cerr << "Unable to open file: \"" << FileName.c_str() << "\"" << endl;
        cerr << "Check directory settings." << endl;
        return -1;
    }

    int InRegisterArguments = ReadInRegisterArguments(SystableStb);

    // duplicate stub files
    DuplicateStubFile(SysstubsStb, SysstubsAsm);
    DuplicateStubFile(SystableStb, SystableAsm);

    SysstubsStb.close();
    SystableStb.close();

    vector<int> ArgTable;
    int ArgCount, ServiceNumber = 0;

    // iterate over the system service table

    for (ServiceList::iterator iter = serviceList.begin(); iter != serviceList.end(); iter++)
    {
        //
        // Store the number of bytes needed for this service 
        //
        ArgCount = (iter->nArguments > InRegisterArguments) ? (iter->nArguments - InRegisterArguments) : 0;
        ArgTable.push_back(ArgCount << 2);

        // 
        // fill sysstubs.asm. For each system service, there are 8 independent macros.
        //
        for (int i = 0; i < 8; i++)
        {
            SysstubsAsm << "SYSSTUBS_ENTRY" << (i + 1) << " " << ServiceNumber << ", ";
            SysstubsAsm << iter->Name->c_str() << ", ";
            SysstubsAsm << ArgCount << endl;
        }

        //
        // write the service to the service table
        //

        SystableAsm << "TABLE_ENTRY " << iter->Name->c_str() << ", ";
        SystableAsm << (ArgCount != 0 ? 1 : 0) << ", ";
        SystableAsm << ArgCount << endl;

        ServiceNumber ++;
    }

    SysstubsAsm << "STUBS_END" << endl;
    SysstubsAsm.close();

    //
    // Specify the number of system services
    //
    SystableAsm << endl << "TABLE_END " << (ServiceNumber - 1) << endl << endl;

    //
    // Create the table of argument bytes
    //
    SystableAsm << "ARGTBL_BEGIN" << endl;
    for (vector<int>::size_type i = 0; i < ArgTable.size(); i += 8)
    {
        SystableAsm << "ARGTBL_ENTRY ";
        for (vector<int>::size_type j = 0; j < 8; j ++)
        {
            if (j != 0)
                SystableAsm << ",";

            if (i+j < ArgTable.size())
            {
                SystableAsm << ArgTable[i + j];
            }
            else
            {
                SystableAsm << 0;
            }

        }

        SystableAsm << endl;
    }

    SystableAsm << endl << "ARGTBL_END" << endl;
    SystableAsm.close();


    return 0;
}