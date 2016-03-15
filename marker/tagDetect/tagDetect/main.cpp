#include <DetectChilitags.hpp>
// This header provides an easy way to use the tag detection information
#include <Chilitag.hpp>
#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <stdlib.h>

using namespace CvConvenience; 
using namespace std;

chilitags::DetectChilitags* tDetector;
IplImage* InputImage;
IplImage* tOutputImage;

// These constants will be used be given to OpenCv for drawing with
// sub-pixel accuracy with fixed point precision coordinates
static const int scShift = 16;
static const float scPrecision = 1 << scShift;
static const int blockNum = 1024;
static const int COUNTER = 3;
// A buffer for the various string formatting
char tTextBuffer[256];
char tRecordTag[1024];

CvFont mFont;
const static CvScalar sColor= CV_RGB(255, 0, 255);
CvPoint2D32f bCenter[1024];
CvPoint2D32f bDir[1024];
int bTag[1024];

#pragma comment(lib, "ws2_32.lib")

#define PORT 4000
#define IP_ADDRESS "127.0.0.1"

int times = 0;

DWORD WINAPI ClientThread(LPVOID lpParameter)
{
	SOCKET ClientSocket = (SOCKET)lpParameter;
	char recvBuffer[MAX_PATH];
	char message[3][MAX_PATH];
	
	int Ret = 0;
	memset(recvBuffer, 0x00, sizeof(recvBuffer));
	Ret = recv(ClientSocket, recvBuffer, MAX_PATH, 0);
	if (Ret == 0 || Ret == SOCKET_ERROR)
	{
		cout << "client quit\n";
		return 0;
	}
	cout << "receive message: " << recvBuffer << endl;
	if (times < COUNTER)
		strcpy(message[times], recvBuffer);
	times++;

	while (times < COUNTER) {}

	printf("times: %d\n", times);

	bool flag = true;
	for(int i = 0; i < COUNTER; i++)
	{
		if (!strcmp(message[i], "false"))
		{
			flag = false;
			break;
		}
	}

	if(flag)
	{
		Ret = send(ClientSocket, "Right\n", 7, 0);
		printf("send something\n");
		if(Ret == SOCKET_ERROR)
		{
			cout << "send info error::" << GetLastError() << endl;
			return 0;
		}
	}
	else
	{
		Ret = send(ClientSocket, "Wrong\n", 7, 0);
		if(Ret == SOCKET_ERROR)
		{
			cout << "send info error::" << GetLastError() << endl;
			return 0;
		}
	}
	return 0;
}	

void swap(int i, int j) {
	int tmp1;
	tmp1 = bTag[i];
	bTag[i] = bTag[j];
	bTag[j] = tmp1;
	CvPoint2D32f tmp2;
	tmp2 = bCenter[i];
	bCenter[i] = bCenter[j];
	bCenter[j] = tmp2;
	CvPoint2D32f tmp3;
	tmp3 = bDir[i];
	bDir[i] = bDir[j];
	bDir[j] = tmp3;
}

void detectMarker(IplImage* InputImage) {
	tDetector->update();

	memcpy(tOutputImage->imageData, InputImage->imageData, InputImage->widthStep * InputImage->height);

	//int index = 0;
	//bool whether_first = false;
	int counter = 0;
	for (int tTagId = 0; tTagId < 1024; ++tTagId) {

		// The Chilitag class is a convenience handle to acces information
		// related to a given tag.
		// The object itself is lightweight, so we can create and delete it
		// frequently (we don't need to store it as member for example)
		chilitags::Chilitag tTag(tTagId,2);

		// Chilitag allows us to easily access the two main pieces of data
		// First, the isPresent() method tells us whether the related tag
		// has been detected in the last frame.
		// This is a first and necessary step to access further information
		// about the tag, as a "absent" tag will have obsolete information.
		if (tTag.isPresent()) 
		{
			//if(whether_first == false)
			//{
			//	index += std::sprintf(tRecordTag+index,"Tag ");
			//	whether_first = true;
			//}
			//printf("%d ",tTagId);
			chilitags::Quad tCorners = tTag.getCorners();

			// We start by drawing this quadrilateral
			for (size_t i = 0; i < chilitags::Quad::scNPoints; ++i) {
				cvLine(
					tOutputImage,
					cvPointFrom32f(scPrecision * tCorners[i]),
					cvPointFrom32f(scPrecision * tCorners[(i+1)%4]),
					sColor, 1, CV_AA, scShift);
			}

			// The quadrilateral is given under the form of a Quad class,
			// which provide a minimal set of geometrical functionalities,
			// such as getCenter()
			CvPoint2D32f tCenter = tCorners.getCenter();
			// We will print the identifier of the tag at its center
			std::sprintf(tTextBuffer, "%d, (%.1f, %.1f)", tTagId, tCenter.x, tCenter.y);
			cvPutText(tOutputImage, tTextBuffer, cvPoint((int)tCenter.x, (int)tCenter.y),
				&mFont, sColor);
			//printf("(%d,%f,%f)\n",tTagId,tCenter.x,tCenter.y);
			//index += std::sprintf(tRecordTag+index,"%d ",tTagId);
			//index += std::sprintf(tRecordTag+index,"%.0f ",tCenter.x);
			//index += std::sprintf(tRecordTag+index,"%.0f ",tCenter.y);
			// Other points an be computed from the four corners of the Quad.
			// Chilitags are oriented. It means that the points 0,1,2,3 of
			// the Quad coordinates are consistently the top-left, top-right,
			// bottom-right and bottom-left
			// (i.e. clockwise, starting from top-left)
			// Using this, we can compute (an approximation of) the middle of
			// the top side of the tag...
			CvPoint2D32f tTop = 0.5f*(tCorners[0] + tCorners[1]);
			// and of its right side
			CvPoint2D32f tRight = 0.5f*(tCorners[1] + tCorners[2]);
			bCenter[counter] = tCenter;
			bTag[counter] = tTagId;
			bDir[counter] = tRight;
			counter++;

			// We display the length in pixel of these sides
			//std::sprintf(tTextBuffer, "The top border is %.1fpx long.",
			//	dist(tCorners[0], tCorners[1]));
			//cvPutText(tOutputImage, tTextBuffer, cvPoint(tTop.x, tTop.y),
			//	&mFont, sColor);
			std::sprintf(tTextBuffer, "rb:%.1f",
				dist(tCorners[1], tCorners[2]));
			cvPutText(tOutputImage, tTextBuffer, cvPoint((int)tRight.x, (int)tRight.y),
				&mFont, sColor);

			// And we draw a line from the center to the midlle of these sides,
			// to show the orientation of the tag.
			/*cvLine(tOutputImage,
				cvPointFrom32f(scPrecision*tCenter),
				cvPointFrom32f(scPrecision*tTop),
				sColor, 1, CV_AA, scShift);
			cvLine(tOutputImage,
				cvPointFrom32f(scPrecision*tCenter),
				cvPointFrom32f(scPrecision*tRight),
				sColor, 1, CV_AA, scShift);*/
		}
	}
	//if(whether_first == true)
	//{
	//	printf("%s\n",tRecordTag);
	//}
	bool flag = true;
	bool vertical = true;
	bool horizon = true;
	if(counter != COUNTER) flag = false;
	for(int i = 0; i < counter-1; i++) {
		for(int j = i + 1; j < counter; j++) {
			if(bTag[i] > bTag[j]) swap(i, j);
		}
	}

	for(int i = 0; i < counter-1; i++) {
		if(bTag[i+1]-bTag[i] > 1) {
			flag = false;
			break;
		}
	}

	for(int i = 0; i < counter-1; i++) {
		if(abs(bCenter[i+1].x-bCenter[i].x) > 20) vertical = false;
		if(abs(bCenter[i+1].y-bCenter[i].y) > 20) horizon = false;
		if(horizon==false && vertical==false) {
			flag = false;
			break;
		}
	}
	if(vertical == true) {
		for(int i = 0; i < counter-1; i++) {
			if(bCenter[i+1].y < bCenter[i].y) {
				flag = false;
				break;
			}
			if(abs(bDir[i+1].x-bDir[i].x) > 20) {
				flag = false;
				break;
			}
		}
	}
	if(horizon == true) {
		for(int i = 0; i < counter-1; i++) {
			if(bCenter[i+1].x < bCenter[i].x) {
				flag = false;
				break;
			}
			if(abs(bDir[i+1].y-bDir[i].y) > 20) {
				flag = false;
				break;
			}
		}
	}

	if(counter > 1) {
		if(flag) {
			printf("Right\n");
			printf("counter:%d\n", counter);
			std::sprintf(tTextBuffer, "Right");
			cvPutText(tOutputImage, tTextBuffer, cvPoint(320, 460),
				&mFont, sColor);
		} else {
			printf("False\n");
			std::sprintf(tTextBuffer, "False");
			cvPutText(tOutputImage, tTextBuffer, cvPoint(320, 460),
				&mFont, sColor);
		}
	}
	cvShowImage("DisplayChilitags",tOutputImage);
	printf("what the hell??????\n");
	fflush(stdout);
}

void init() {
	tDetector = new chilitags::DetectChilitags(&InputImage);
	CvSize sz;
	sz.width = 640;
	sz.height = 480;
	InputImage = cvCreateImage(sz,IPL_DEPTH_8U,3);
	tOutputImage = cvCreateImage(sz,IPL_DEPTH_8U,3);

	cvInitFont(&mFont,CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
	cvNamedWindow("DisplayChilitags");
}

int main(int argc,char* argv[])
{
	init();
	WSADATA  Ws;
	SOCKET ServerSocket, ClientSocket;
	struct sockaddr_in LocalAddr, ClientAddr;
	int Ret = 0;
	int AddrLen = 0;
	HANDLE hThread = NULL;

	//Init Windows Socket
	if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
	{
		cout<<"Init Windows Socket Failed::"<<GetLastError()<<endl;
		return -1;
	}

	//Create Socket
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( ServerSocket == INVALID_SOCKET )
	{
		cout<<"Create Socket Failed::"<<GetLastError()<<endl;
		return -1;
	}

	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	LocalAddr.sin_port = htons(PORT);
	memset(LocalAddr.sin_zero, 0x00, 8);

	//Bind Socket
	Ret = bind(ServerSocket, (struct sockaddr*)&LocalAddr, sizeof(LocalAddr));
	if ( Ret != 0 )
	{
		cout<<"Bind Socket Failed::"<<GetLastError()<<endl;
		return -1;
	}
	//listen
	Ret = listen(ServerSocket, 10);
	if ( Ret != 0 )
	{
		cout<<"listen Socket Failed::"<<GetLastError()<<endl;
		return -1;
	}

	cout<<"server start..."<<endl;


	CvCapture* cap;
	cap = cvCaptureFromCAM(0);
	if(!cap)
	{
		cout<<"create camera capture error"<<endl;
		system("pause");
		exit(-1);
	}
	//cvNamedWindow("img",1);

	while(1)
	{
		AddrLen = sizeof(ClientAddr);
		ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddr, &AddrLen);
		if ( ClientSocket == INVALID_SOCKET )
		{
			cout<<"Accept Failed::"<<GetLastError()<<endl;
			break;
		}
		cout<<"client connect::"<<inet_ntoa(ClientAddr.sin_addr)<<":"<<ClientAddr.sin_port<<endl;

		InputImage = cvQueryFrame(cap);
		if(!InputImage)
			break;
		//cvShowImage("img",InputImage);
		cvWaitKey(1);
		detectMarker(InputImage);
		hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)ClientSocket, 0, NULL);
		if ( hThread == NULL )
		{
			cout<<"Create Thread Failed!"<<endl;
			break;
		}
		CloseHandle(hThread);
	}
	cvReleaseCapture(&cap);
	cvDestroyAllWindows();
	cvReleaseImage(&InputImage);
	return 0;
}