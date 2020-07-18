#include<opencv2\opencv.hpp>
#include<iostream>

using namespace std;

float diff(CvScalar a, CvScalar b)   // 색간 거리를 계산하는 함수
{
	float d = (a.val[0] - b.val[0])*(a.val[0] - b.val[0])
		+ (a.val[1] - b.val[1])*(a.val[1] - b.val[1])
		+ (a.val[2] - b.val[2])*(a.val[2] - b.val[2]);
	return sqrt(d);
}

CvScalar ** makeSAT(IplImage * src)
{
	CvSize size = cvGetSize(src);   //   원본이미지의 크기를 받아온다
	CvScalar ** dataChart = new CvScalar *[size.height];   //   SAT의 행수만큼 생성
	for (int i = 0; i < size.height; i++)
		dataChart[i] = new CvScalar[size.width];   //   SAT의 열수만큼 생성

	for (int y = 0; y < size.height; y++)
	{
		CvScalar s = cvGet2D(src, y, 0);
		dataChart[y][0].val[0] = s.val[0];
		dataChart[y][0].val[1] = s.val[1];
		dataChart[y][0].val[2] = s.val[2];

		for (int x = 1; x < size.width; x++)
		{
			CvScalar s = cvGet2D(src, y, x);
			dataChart[y][x].val[0] = dataChart[y][x - 1].val[0] + s.val[0];
			dataChart[y][x].val[1] = dataChart[y][x - 1].val[1] + s.val[1];
			dataChart[y][x].val[2] = dataChart[y][x - 1].val[2] + s.val[2];

		}
	}   // Big O(n^2)

	for (int y = 1; y < size.height; y++)
	{
		for (int x = 0; x < size.width; x++)
		{
			dataChart[y][x].val[0] = dataChart[y - 1][x].val[0] + dataChart[y][x].val[0];
			dataChart[y][x].val[1] = dataChart[y - 1][x].val[1] + dataChart[y][x].val[1];
			dataChart[y][x].val[2] = dataChart[y - 1][x].val[2] + dataChart[y][x].val[2];
		}   // Big O(n^2)
	}

	return dataChart;
}

void getDiff(IplImage * src, IplImage * blur, IplImage * factor)
{
	// f - x = f'
	// 원본 이미지에서 잃어버린것
	// f = f' + x
	// x = f - f'...
	// I want to get 'x'!
	cvSet(factor, cvScalar(255, 255, 255));

	CvSize size = cvGetSize(src);
	for (int y = 0; y<size.height; y++)
		for (int x = 0; x < size.width; x++)
		{
			CvScalar f1 = cvGet2D(src, y, x);
			CvScalar f2 = cvGet2D(blur, y, x);
			CvScalar g;
			// 검었던 부분은 뭉개면 하얀색으로. 그러니까 차이는 결과적으로 음수쪽으로 
			// 밝았던 부분은 뭉개면 어둡게. 그러니까 차이는 결과적으로 양수로.
			g.val[0] = f1.val[0] - f2.val[0];
			g.val[1] = f1.val[1] - f2.val[1];
			g.val[2] = f1.val[2] - f2.val[2];
			cvSet2D(factor, y, x, g);
		}
}

int main(void)
{
	//	1. image sharpen을 구현한다
	//	2. image thresholding & 경계값 지정을 구현한다

	cout << "========================================" << endl;
	cout << "Dept. of Digital Contents, Sejong University" << endl;
	cout << "Multimedia Programming Class" << endl;
	cout << "Homework #4: Shadow Remover" << endl;
	cout << "========================================" << endl;
	cout << "input Filename:";
	
	char file_name[100];
	cin >> file_name;

	IplImage * src = cvLoadImage(file_name);	
	CvSize size = cvGetSize(src);	
	IplImage * dst = cvCreateImage(size, 8, 3);
	
	double myMask[3][3] =
	{ { 0, -1,0 },
	{ -1,5, -1 },
	{ 0,-1, 0 } };

	// -1 -1 -1
	// -1  9 -1
	// -1 -1 -1
	// -> laplacian kernel

	// laplacian mask VS mask1 & mask2 ? VS myMask
	int kernel_size = 1;
	//실수한 부분 : size를 계속 잘못 주고 있었음
	//CvMat kernel = cvMat(3, 3, CV_64FC1, mask1);
	//cvFilter2D(src, dst, &kernel);

	for (int y = kernel_size; y<size.height - kernel_size; y++)
		for (int x = kernel_size; x<size.width - kernel_size; x++)
		{
			CvScalar g = cvScalar(0, 0, 0);
			for (int v = -kernel_size; v <= kernel_size; v++)
				for (int u = -kernel_size; u <= kernel_size; u++)
				{
					CvScalar f = cvGet2D(src, y + v, x + u);
					g.val[0] += f.val[0] * myMask[v + kernel_size][u + kernel_size];
					g.val[1] += f.val[1] * myMask[v + kernel_size][u + kernel_size];
					g.val[2] += f.val[2] * myMask[v + kernel_size][u + kernel_size];
				}
			cvSet2D(dst, y, x, g);
		}

	// Image thresholding + alpha...
	CvScalar palette[2];
	palette[0] = cvScalar(0, 0, 0);
	palette[1] = cvScalar(255, 255, 255);
	int numColor = 2;
	int k = 3;
	CvScalar ** dataChart = makeSAT(dst);
	for (int y = k + 1; y<size.height - k - 1; y++)
		for (int x = k + 1; x<size.width - k - 1; x++)
		{
			CvScalar f = cvGet2D(dst, y, x);
			float min_d = FLT_MAX;
			int min_color;

			CvScalar g;
			g.val[0] = (dataChart[y + k][x + k].val[0]
				- dataChart[y - k - 1][x + k].val[0]
				- dataChart[y + k][x - k - 1].val[0]
				+ dataChart[y - k - 1][x - k - 1].val[0])
				/ double((2 * k + 1)*(2 * k + 1));

			g.val[1] = (dataChart[y + k][x + k].val[1]
				- dataChart[y - k - 1][x + k].val[1]
				- dataChart[y + k][x - k - 1].val[1]
				+ dataChart[y - k - 1][x - k - 1].val[1])
				/ double((2 * k + 1)*(2 * k + 1));

			g.val[2] = (dataChart[y + k][x + k].val[2]
				- dataChart[y - k - 1][x + k].val[2]
				- dataChart[y + k][x - k - 1].val[2]
				+ dataChart[y - k - 1][x - k - 1].val[2])
				/ double((2 * k + 1)*(2 * k + 1));

			palette[1] = g;
			float avg = (g.val[0] + g.val[1] + g.val[2]) / 3;
			for (int i = 0; i < numColor; i++)
			{
				float d = diff(f, palette[i]);
				if (d < min_d)
				{
					min_color = i;
					min_d = d;
				}
			}
			if (min_color == 1)
				palette[1] = cvScalar(255, 255, 255);

			cvSet2D(dst, y, x, palette[min_color]);
		}
	

	//	이미지를 저장하는 부분
	file_name[0] = 'r';
	cvSaveImage(file_name, dst);


	cvShowImage("src", src);
	cvShowImage("dst", dst);

	cvWaitKey();
	return 0;
}

//   가장 많이 쓰인 색을 흰색
//   가장 적고 어둡게 칠한 색을 검정으로. ... 히스토그램 이용

//   이 점과 주변 점들의 평균을 비교해서 일정경계값 이상이면 흰색 이하면 검정.
//   근데 여기선 경계값 설정하기가 어렵다
//   경계값은 주변이 어두울수록 낮고 주변이 밝다면 그대로. 

//   image thresholding 한번만 사용해도 대강은 나오긴하지만....
//   어느 그림자 영역이면 전부 검게 나옴->> 사전작업이 필요함
