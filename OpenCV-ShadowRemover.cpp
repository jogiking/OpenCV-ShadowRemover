#include<opencv2\opencv.hpp>
#include<iostream>

using namespace std;

float diff(CvScalar a, CvScalar b)   // ���� �Ÿ��� ����ϴ� �Լ�
{
	float d = (a.val[0] - b.val[0])*(a.val[0] - b.val[0])
		+ (a.val[1] - b.val[1])*(a.val[1] - b.val[1])
		+ (a.val[2] - b.val[2])*(a.val[2] - b.val[2]);
	return sqrt(d);
}

CvScalar ** makeSAT(IplImage * src)
{
	CvSize size = cvGetSize(src);   //   �����̹����� ũ�⸦ �޾ƿ´�
	CvScalar ** dataChart = new CvScalar *[size.height];   //   SAT�� �����ŭ ����
	for (int i = 0; i < size.height; i++)
		dataChart[i] = new CvScalar[size.width];   //   SAT�� ������ŭ ����

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
	// ���� �̹������� �Ҿ������
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
			// �˾��� �κ��� ������ �Ͼ������. �׷��ϱ� ���̴� ��������� ���������� 
			// ��Ҵ� �κ��� ������ ��Ӱ�. �׷��ϱ� ���̴� ��������� �����.
			g.val[0] = f1.val[0] - f2.val[0];
			g.val[1] = f1.val[1] - f2.val[1];
			g.val[2] = f1.val[2] - f2.val[2];
			cvSet2D(factor, y, x, g);
		}
}

int main(void)
{
	//	1. image sharpen�� �����Ѵ�
	//	2. image thresholding & ��谪 ������ �����Ѵ�

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
	//�Ǽ��� �κ� : size�� ��� �߸� �ְ� �־���
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
	

	//	�̹����� �����ϴ� �κ�
	file_name[0] = 'r';
	cvSaveImage(file_name, dst);


	cvShowImage("src", src);
	cvShowImage("dst", dst);

	cvWaitKey();
	return 0;
}

//   ���� ���� ���� ���� ���
//   ���� ���� ��Ӱ� ĥ�� ���� ��������. ... ������׷� �̿�

//   �� ���� �ֺ� ������ ����� ���ؼ� ������谪 �̻��̸� ��� ���ϸ� ����.
//   �ٵ� ���⼱ ��谪 �����ϱⰡ ��ƴ�
//   ��谪�� �ֺ��� ��ο���� ���� �ֺ��� ��ٸ� �״��. 

//   image thresholding �ѹ��� ����ص� �밭�� ������������....
//   ��� �׸��� �����̸� ���� �˰� ����->> �����۾��� �ʿ���
