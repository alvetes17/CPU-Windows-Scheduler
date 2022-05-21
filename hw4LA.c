/*
Luis Alvear
*/
#include <windows.h>
#include <stdio.h>

typedef struct processor_data {
   int affinityMask;                /* affinity mask of this processor (just one bit set) */
   PROCESS_INFORMATION processInfo; /* process currently running on this processor */
   int running;                     /* 1 when this processor is running a task, 0 otherwise */
} ProcessorData;


/* function prototypes */
void printError(char* functionName);
void creatingProcess(int argc, char *argv[], ProcessorData *processorPool, char commandLine[], ProcessorData *processInfo);

int main(int argc, char *argv[])
{
   int *times;                   /* array to hold job duration times from the command-line */
   int processorCount = -1;           /* the number of allocated processors, from GetProcessAffinityMask */
   ProcessorData *processorPool; /* an array of ProcessorData structures */
   HANDLE *processHandles;       /* an array of handles to processes, needed by WaitForMultipleObjects */
   int *handleIndexInPool;       /* parallel array, where each handle comes from in processorPool */
   int handleCount;
   DWORD processId = GetCurrentProcessId();
   HANDLE hProcess = GetCurrentProcess();
   DWORD_PTR processAffinityMask;
   DWORD_PTR  systemAffinityMask;
   
   /*STARTUPINFO startInfo;
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);*/
   
   char commandLine[100];
   PROCESS_INFORMATION *processInfo = malloc( (argc-1) * sizeof(PROCESS_INFORMATION) );

   if (argc < 3)
   {
      fprintf(stderr, "usage, %s  SCHEDULE_TYPE  SECONDS...\n", argv[0]);
      fprintf(stderr, "Where: SCHEDULE_TYPE = 0 means \"first come first serve\"\n");
      fprintf(stderr, "       SCHEDULE_TYPE = 1 means \"shortest job first\"\n");
      fprintf(stderr, "       SCHEDULE_TYPE = 2 means \"longest job first\"\n");
      return 0;
   }

   /* Read the schedule type off the command-line. */
   if (atoi(argv[1]) == 0 )
   {   
   }
   if (atoi(argv[1]) == 1)
   {
      for (int i = 2 ; i < argc; ++i)
      {
         for (int j = i + 1; j < argc; ++j)
         {
            if (atoi(argv[i]) > atoi(argv[j]))
            {
               int temp = argv[i];
               argv[i] = argv[j];
               argv[j] = temp;
            }
         }
      }   
   }
   if (atoi(argv[1]) == 2)
   {
      for (int i = 2 ; i < argc; ++i)
      {
         for (int j = i + 1; j < argc; ++j)
         {
            if (atoi(argv[i]) < atoi(argv[j]))
            {
               int temp = argv[i];
               argv[i] = argv[j];
               argv[j] = temp;
            }
        }  
      }
   }
   
   /* Read the job duration times off the command-line. */
   for (int i =2; i < argc; i++){
      times = atoi(argv[i]);
   }
   
   /* Get the processor affinity mask for this process. */
   GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask);

   /* Count the number of processors set in the affinity mask. */
   DWORD counter = (DWORD) processAffinityMask;
   for( int i = 0; i < 16; i++){
      if( counter >> i & 1) 
      {
         processorCount += 1;
      }
   }

   /* Create, and then initialize, the processor pool array of data structures. */
   processorPool = malloc(processorCount * sizeof(ProcessorData));
   
   for( int i = 0; i < argc; i++){
      processorPool[i].affinityMask = 0b00000001<< i;
      processorPool[i].running = 0;
   }
   
   printf("processAffinityMask=%#6.4x, processorCount=%d, processId=%d\n\n", (int)processAffinityMask, ((int)processorCount +1), (int)processId);
   
   /* Start the first group of processes. */  
   
   printf("Scheduling job duration times: ");
   for (int i = 2; i < argc; ++i)
   {
      printf("%d ", atoi(argv[i]));
   }
   printf("\n");
   
   creatingProcess(argc, argv, processorPool, commandLine, &processInfo);

   /* Repeatedly wait for a process to finish and then, if there are more jobs to run, run a new job on the processor that just became free. */
   for (int i = 0; i < argc; i++){
      WaitForSingleObject(processInfo[i].hProcess, INFINITE);
   }
   
  while (1)
   {
      /* Get, from the processor pool, handles to the currently running processes. */
      /* Put those handles in an array. */
      int i=0;
      processHandles[i]=processorPool[i].processInfo.hProcess;
      
      /* Use a parallel array to keep track of where in the processor pool each handle came from. */
      for (int i=0; i< argc; i++)
      {
      handleIndexInPool[i]=processHandles[i];
      }
      
      /* Check that there are still processes running, if not, quit. */
      /* Wait for one of the running processes to end. */
      DWORD result;
      if (WAIT_FAILED == (result = WaitForMultipleObjects(handleCount, processHandles, FALSE, INFINITE)))
      printError("WaitForMultipleObjects");

      /* Translate result from an index in processHandles[] to an index in processorPool[]. */
     
      /* Close the handles of the finished process and update the processorPool array. */
      for (int i = 2; i < argc; i++)
      {
         result=processorPool[i].processInfo.hProcess;
         CloseHandle(processInfo[i].hThread);
         CloseHandle(processInfo[i].hProcess);
      }
      /* Check if there is another process to run on the processor that just became free. */
      
      for (int i=2; i<argc; i++)
      {
         if (processHandles[i]==0)
         {
         processorPool[i].running=1;
         }
         else if (processHandles[i]!=0)
         {
         return 1;
         }
      }
    i++;
   } 

  
   return 0;
}

/*******************************************************************
   This function prints out "meaningful" error messages. If you call
   a Windows function and it returns with an error condition, then
   call this function right away and pass it a string containing the
   name of the Windows function that failed. This function will print
   out a reasonable text message explaining the error.
*/

void creatingProcess(int argc, char *argv[], ProcessorData *processorPool, char commandLine[], ProcessorData *processInfo)
{
   STARTUPINFO startInfo;
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);
for (int i = 2; i < argc; i++){
   
      sprintf(commandLine, "%s", "computeProgram_64.exe");
      sprintf(commandLine, "%s %s", commandLine, argv[i]);
     
      if( !CreateProcess(NULL, commandLine, NULL, NULL, TRUE,
                         CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
                         NULL, NULL, &startInfo, &processorPool[i].processInfo) ){
          printError("CreateProcess");
      }
      else{
         printf("launched %d job on procesor  with mask %#4.2x\n", atoi (argv[i]), (int)processorPool[i].affinityMask);
         SetProcessAffinityMask(processorPool[i].processInfo.hProcess, processorPool[i].affinityMask);
         ResumeThread(processorPool[i].processInfo.hThread);
         processorPool[i].running = 1;
      }
      
   }
}
void printError(char* functionName)
{
   LPSTR lpMsgBuf = NULL;
   int error_no;
   error_no = GetLastError();
   FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      error_no,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
      (LPTSTR)&lpMsgBuf,
      0,
      NULL
   );
   /* Display the string. */
   fprintf(stderr, "\n%s failed on error %d: %s", functionName, error_no, lpMsgBuf);
   //MessageBox(NULL, lpMsgBuf, functionName, MB_OK);
   /* Free the buffer. */
   LocalFree( lpMsgBuf );
}