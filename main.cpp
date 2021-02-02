#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include <winuser.h>
#include <stdlib.h>
#include <dirent.h> 
const int IDT_TIMER1 = 0;
bool running = false;
char file[MAX_PATH];
int DisplayResourceNAMessageBox(HWND,char*,char*);
int isFile(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
     closedir(directory);
     return 0;
    }else return 1;

    return -1;
}
bool copyFiles(char * dir)
{	
	struct dirent *de;
    DIR *dr = opendir(file);
    while((de = readdir(dr)) != NULL) 
    {
		char f[MAX_PATH];
		char o[MAX_PATH];
		strcpy(o,file);
		strcat(o,"\\");
		strcat(o,de->d_name);
		strcpy(f,file);
		strcat(f,"\\");
		strcat(f,de->d_name);
		strcat(f,".backup");
		CopyFile(o,f,false);	
	}
	return true;
}
bool backup(){
	char nfile[MAX_PATH];
	strcpy(nfile, file);
	strcat(nfile,".backup");
	if(isFile(file)){
		if(!CopyFile(file,nfile,false))
		{
			int EC = GetLastError();			
			DisplayResourceNAMessageBox(NULL, "Ocurrio un error en la copia", "ERROR");
			return false;
			}
		return true;	
	}else{
		return copyFiles(file);
	}	
}
/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		
		/* trap the WM_CLOSE (clicking X) message, and actually tell the window to close */
		case WM_CLOSE: {
			DestroyWindow(hwnd);
			break;
		}
		
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		case WM_TIMER: {
			backup();			
			break;
		}
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
void quit(HWND hwnd){
	if (running) KillTimer(hwnd, IDT_TIMER1);
	DestroyWindow(hwnd);
}
int DisplayResourceNAMessageBox(HWND hwnd, char* msg, char* title)
{	
    int msgboxID = MessageBox(
        NULL,
        msg,
        title,
        MB_ICONINFORMATION | MB_OK
    );
    if(msgboxID == IDOK){
		quit(hwnd);
    }

    return msgboxID;
}
/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG Msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(hInstance, "A"); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(hInstance, "A");

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Caption",WS_DISABLED,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		640, /* width */
		480, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	LPWSTR *szArglist;
   	int nArgs;

   	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
   	if( NULL == szArglist )
   	{ 
		DisplayResourceNAMessageBox(hwnd, "Error en parametros", "Error");		     
      	return 0;
   	}   	
   	if(nArgs <= 2)
   		DisplayResourceNAMessageBox(hwnd, "Error en parametros, el primero debe ser la ruta del archivo\nel segundo el intervalo de autoguardado en minutos\nEj. autosaver c:\\myfile.bak 10", "Error");
   	char buffer[3];
   	char path[MAX_PATH];
   	wcstombs(path, szArglist[1], MAX_PATH);
   	wcstombs(buffer, szArglist[2], 3);
   	int time =atoi(buffer);
   	for(int i=0;i<MAX_PATH;i++)
   		if(path[i]!=0x0)
   			file[i]=path[i];
   		else
   			break;
	// Free memory allocated for CommandLineToArgvW arguments.

   	LocalFree(szArglist);
	SetTimer(hwnd,             // handle to main window 
    IDT_TIMER1,            // timer identifier 
    time*1000*60,                 // 10-second interval 
    (TIMERPROC) NULL);     // no timer callback 
    running = true;
	char msg[MAX_PATH];
	strcpy(msg, file);
	strcat(msg,"\nPresione OK para terminar");  
	if(backup())
		DisplayResourceNAMessageBox(hwnd, msg, "AutoSaver Guardando...");
	else
		quit(hwnd);
	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produre unreasonably CPU usage
	*/
	while(GetMessage(&Msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&Msg); /* Translate keycodes to chars if present */
		DispatchMessage(&Msg); /* Send it to WndProc */
	}
	return Msg.wParam;
}

