#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#include <CL/cl.hpp>
#endif

#include <windows.h>
#include <cmath>

//#include <stdio.h>
//#include <stdlib.h>

//#include <string.h> 
//#include <time.h>
//#include <exception>
//#define __CL_ENABLE_EXCEPTIONS
#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)
#define num_of_kernels (4)

void printMatrix(float* M, int row, int col)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
			printf(" %12.3f", M[col*i + j]);
		printf("\n");
	}
}

void  checkRet(cl_int ret)
{
	if (ret)
	{
		printf("FATAL ERROR!! RET:  %i\n", ret);
		//throw std::invalid_argument("setPos illegal argument!");
		system("pause");
		exit(ret);
	}
}
int main()
{
	system("title MatrixMultiply");
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD maxWindow = GetLargestConsoleWindowSize(handle);
	COORD bufferSize = { maxWindow.X * 5, maxWindow.Y * 15 };
	SetConsoleScreenBufferSize(handle, bufferSize);
	HWND hwnd = GetForegroundWindow();
	ShowWindow(hwnd, SW_SHOWMAXIMIZED);


	cl_uint ret_num_platforms;
	cl_uint ret_num_devices;
	cl_int ret = CL_SUCCESS;
	cl_int sel_num = -1;
	cl_device_type device_type;

	char string[MEM_SIZE];
	bool isRight = false;

	FILE *fp;
	char fileName[] = "./Kernels.cl";
	char *source_str;
	size_t source_size;

	fp = fopen(fileName, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(EXIT_FAILURE);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	checkRet(clGetPlatformIDs(0, NULL, &ret_num_platforms));
	cl_platform_id *platform_id = new cl_platform_id[ret_num_platforms];
	checkRet(clGetPlatformIDs(ret_num_platforms, platform_id, &ret_num_platforms));

	while (!isRight)
	{
		printf("\nFounded %i platform(s). Select one of the following platforms:\n", ret_num_platforms);
		for (int i = 0; i < ret_num_platforms; i++)
		{
			checkRet(clGetPlatformInfo(platform_id[i], CL_PLATFORM_NAME, sizeof(string), &string, NULL));
			printf("\n%i) %s ", i, string);
			checkRet(clGetPlatformInfo(platform_id[i], CL_PLATFORM_VENDOR, sizeof(string), &string, NULL));
			printf("by %s\n", string);
		}
		printf("\nYour choice: ");
		scanf("%i", &sel_num);
		isRight = sel_num >= 0 && sel_num < ret_num_platforms;
		if (!isRight)
		{
			printf("\nError. Invalid platform number. Try again.\n");
			rewind(stdin);
		}
	};
	checkRet(clGetDeviceIDs(platform_id[sel_num], CL_DEVICE_TYPE_ALL, 0, NULL, &ret_num_devices));
	cl_device_id *device_id = new cl_device_id[ret_num_devices];
	checkRet(clGetDeviceIDs(platform_id[sel_num], CL_DEVICE_TYPE_ALL, ret_num_devices, device_id, &ret_num_devices));
	checkRet(clGetPlatformInfo(platform_id[sel_num], CL_PLATFORM_NAME, sizeof(string), &string, NULL));

	isRight = false;

	cl_uint dimensions = 0;
	while (!isRight)
	{
		printf("\nThe list of devices available on %s platform:\n", string);
		for (int i = 0; i < ret_num_devices; i++)
		{
			checkRet(clGetDeviceInfo(device_id[i], CL_DEVICE_NAME, sizeof(string), &string, NULL));
			printf("\n%i) %s ", i, string);
			checkRet(clGetDeviceInfo(device_id[i], CL_DEVICE_VERSION, sizeof(string), &string, NULL));
			printf("which supports %s ", string);
			checkRet(clGetDeviceInfo(device_id[i], CL_DEVICE_VENDOR, sizeof(string), &string, NULL));
			printf("by %s ", string);
			checkRet(clGetDeviceInfo(device_id[i], CL_DEVICE_TYPE, sizeof(device_type), &device_type, NULL));
			
			switch (device_type)
			{
			case CL_DEVICE_TYPE_CPU:
				printf("(CPU)");
				break;
			case CL_DEVICE_TYPE_GPU:
				printf("(GPU)");
				break;
			case CL_DEVICE_TYPE_ACCELERATOR:
				printf("(Accelerator)");
				break;
			case CL_DEVICE_TYPE_DEFAULT:
				printf("(Default)");
				break;
			case CL_DEVICE_TYPE_CUSTOM:
				printf("(Custom)");
				break;
			}
			checkRet(clGetDeviceInfo(device_id[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(dimensions), &dimensions, NULL));
			cl_uint *NDRange = new cl_uint[dimensions];
			checkRet(clGetDeviceInfo(device_id[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, _msize(NDRange), NDRange, NULL));

			printf(" \nNDRange: (");
			for (int i = 0; i < dimensions; i++)
			{
				printf("%u", NDRange[i]);
				if (i < dimensions - 1)
					printf(", ");
			}
			printf(")\n");
		}

		printf("\nYour choice: ");
		scanf("%i", &sel_num);
		isRight = sel_num >= 0 && sel_num < ret_num_devices;
		if (!isRight)
		{
			printf("\nError. Invalid device number. Try again.\n");
			rewind(stdin);
		}
	};

	checkRet(clGetDeviceInfo(device_id[sel_num], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(dimensions), &dimensions, NULL));
	cl_uint *NDRange = new cl_uint[dimensions];
	checkRet(clGetDeviceInfo(device_id[sel_num], CL_DEVICE_MAX_WORK_ITEM_SIZES, _msize(NDRange), NDRange, NULL));

	isRight = false;
	cl_uint row1 = 0, row2 = 0, col1 = 0, col2 = 0;
	while (!isRight)
	{
		printf("\nEnter the dimensions of the first matrix (two numbers): ");
		scanf("%u%u", &row1, &col1); 
		printf("\nEnter the dimensions of the second matrix (two numbers): ");
		scanf("%u%u", &row2, &col2);  
		//printf("\a");
		isRight = col1 == row2;
		if (!isRight)
			printf("\nError. I still can't multiply this matrices. Let's start from the beginning.");
	}

	float *A = new float[row1*col1];
	float *B = new float[row2*col2];
	float *C = new float[row1*col2];
	float *D = new float[row1*col2];
	float *E = new float[row1*col2];
	float *F = new float[row1*col2];
	float *G = new float[row1*col2];
	//float *Matrix = new float[row1*col2, num_of_kernels + 1];
	//srand(time(NULL));
	for (int i = 0; i < row1; i++)
	{
		for (int j = 0; j < col1; j++)
		{
			A[col1*i + j] = 1000 * ((float)rand() / RAND_MAX);
			//A[col1*i + j] = ((rand() % 1000));
		}
	}

	for (int i = 0; i < row2; i++)
	{
		for (int j = 0; j < col2; j++)
		{
			B[col2*i + j] = 1000 * ((float)rand() / RAND_MAX);
			//B[col2*i + j] = ((rand() % 1000));
		}
	}

	for (int i = 0; i < row1; i++)
	{
		for (int j = 0; j < col2; j++)
		{
			C[col2*i + j] = 0;
			D[col2*i + j] = 0;
			E[col2*i + j] = 0;
			F[col2*i + j] = 0;
			G[col2*i + j] = 0;
			for (int k = 0; k < col1; k++)
				C[col2*i + j] += A[col1*i + k] * B[col2*k + j];
		}
	}

	cl_context context = clCreateContext(NULL, 1, &device_id[sel_num], NULL, NULL, &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);

	cl_command_queue command_queue = clCreateCommandQueue/*WithProperties*/(context, device_id[sel_num], CL_QUEUE_PROFILING_ENABLE /*| CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE*/  /*0*/, &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);

	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);
	checkRet(clBuildProgram(program, 1, &device_id[sel_num], NULL, NULL, NULL));

	cl_mem arg0 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, _msize(A), A, &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);
	cl_mem arg1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, _msize(B), B, &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);
	cl_mem arg2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY/*CL_MEM_READ_WRITE */ | CL_MEM_COPY_HOST_PTR /*или не хост птр*/, _msize(D), D, &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);


	cl_event *events = new cl_event[num_of_kernels];
	cl_kernel *kernels = new cl_kernel[num_of_kernels];
	kernels[0] = clCreateKernel(program, "MatrixMultiply0", &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);
	kernels[1] = clCreateKernel(program, "MatrixMultiply1", &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);
	kernels[2] = clCreateKernel(program, "MatrixMultiply2", &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);
	kernels[3] = clCreateKernel(program, "MatrixMultiply3", &ret);
	if (ret) printf("FATAL ERROR!! RET:  %i\n", ret);

	
	for (int i = 0; i < num_of_kernels; i++)
	{
		//kernels[i] = clCreateKernel(program, "MatrixMultiply"+symbol1+'0'/*+i+'0'*//*+std::to_string(i)*/, &ret);
		checkRet(clSetKernelArg(kernels[i], 0, sizeof(cl_mem), (void *)&arg0));
		checkRet(clSetKernelArg(kernels[i], 1, sizeof(cl_mem), (void *)&arg1));
		checkRet(clSetKernelArg(kernels[i], 2, sizeof(cl_mem), (void *)&arg2));
		checkRet(clSetKernelArg(kernels[i], 3, sizeof(cl_int), (void *)&col1));
		checkRet(clSetKernelArg(kernels[i], 4, sizeof(cl_int), (void *)&col2));
	}
	checkRet(clSetKernelArg(kernels[1], 5, sizeof(cl_int), (void *)&row1));

	/*ZERO KERNEL*/
	size_t global_work_size0[1] = { row1 };
	checkRet(clEnqueueWriteBuffer(command_queue, arg2, CL_TRUE, 0, _msize(D), D, 0, NULL, NULL));
	checkRet(clEnqueueNDRangeKernel(command_queue, kernels[0], 1, NULL, global_work_size0, /*global_work_size*/ NULL, 0, NULL, &events[0] /*NULL*/));
	checkRet(clEnqueueReadBuffer(command_queue, arg2, CL_TRUE, 0, _msize(D), D, /*1, &event*/0, NULL, NULL));
	//D =(float*) clEnqueueMapBuffer(command_queue, arg2,CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, _msize(D) ,0, NULL, NULL, &ret);

	/*FIRST KERNEL*/
	size_t global_work_size1[1] = { col2 };
	checkRet(clEnqueueWriteBuffer(command_queue, arg2, CL_TRUE, 0, _msize(E), E, 0, NULL, NULL));
	checkRet(clEnqueueNDRangeKernel(command_queue, kernels[1], 1, NULL, global_work_size1, /*global_work_size*/ NULL, 0, NULL, &events[1] /*NULL*/));
	checkRet(clEnqueueReadBuffer(command_queue, arg2, CL_TRUE, 0, _msize(E), E, 0, NULL, NULL));

	/*SECOND KERNEL*/
	size_t global_work_size2[2] = { row1, col2 };
	checkRet(clEnqueueWriteBuffer(command_queue, arg2, CL_TRUE, 0, _msize(F), F, 0, NULL, NULL));
	checkRet(clEnqueueNDRangeKernel(command_queue, kernels[2], 2, NULL, global_work_size2, /*global_work_size*/ NULL, 0, NULL, &events[2] /*NULL*/));
	checkRet(clEnqueueReadBuffer(command_queue, arg2, CL_TRUE, 0, _msize(F), F, 0, NULL, NULL));



	//NDRange[0] = 4;
	//NDRange[1] = 4;
	//NDRange[2] = 4;


	/*THIRD KERNEL*/
	checkRet(clEnqueueWriteBuffer(command_queue, arg2, CL_TRUE, 0, _msize(G), G, 0, NULL, NULL));
	int num_of_iter[3] = { ceil((float)row1/ (float)NDRange[0]), ceil((float)col2 / (float)NDRange[1]), ceil((float)col1 / (float)NDRange[2])};
	//size_t global_work_offset3[3];
	size_t global_work_size3[3];
	size_t local_work_size3[3] = { 1, 1, col1 };

	for (int i = 0; i < num_of_iter[0]; i++)
	{
		for (int j = 0; j < num_of_iter[1]; j++)
		{
			for (int k = 0; k < num_of_iter[2]; k++)
			{
				size_t global_work_offset3[3] = { i*NDRange[0], j*NDRange[1],k*NDRange[2] };
				global_work_size3[0] = (i != num_of_iter[0] - 1) ? NDRange[0] : row1 - NDRange[0] * i;
				global_work_size3[1] = (j != num_of_iter[1] - 1) ? NDRange[1] : col2 - NDRange[1] * j;
				global_work_size3[2] = (k != num_of_iter[2] - 1) ? NDRange[2] : col1 - NDRange[2] * k;
				local_work_size3[2] = global_work_size3[2];
				checkRet(clEnqueueNDRangeKernel(command_queue, kernels[3], 3, global_work_offset3 /*NULL*/, global_work_size3, local_work_size3 /*NULL*/, 0, NULL, &events[3] /*NULL)*/));
			}
		}
	}
		checkRet(clEnqueueReadBuffer(command_queue, arg2, CL_TRUE, 0, _msize(G), G, 0, NULL, NULL));

	//size_t global_work_offset3[3];
	//size_t global_work_size3[3] = {row1, col2, col1};
	//size_t local_work_size3[3] = { 1, 1, col1 };
	//checkRet(clEnqueueWriteBuffer(command_queue, arg2, CL_TRUE, 0, _msize(G), G, 0, NULL, NULL));
	//checkRet(clEnqueueNDRangeKernel(command_queue, kernels[3], 3, /*global_work_offset3 */NULL, global_work_size3, local_work_size3 /*NULL*/, 0, NULL, &events[3] /*NULL)*/));
	//checkRet(clEnqueueReadBuffer(command_queue, arg2, CL_TRUE, 0, _msize(G), G, 0, NULL, NULL));


	//Old kernel execution:
	//checkRet(clEnqueueWriteBuffer(command_queue, arg2, CL_TRUE, 0, _msize(G), G, 0, NULL, NULL));
	//size_t global_work_size3[2] = { row1, col2 };
	//for (int i = 0; i < col1; i++)
	//{
	//	checkRet(clSetKernelArg(kernels[3], 5, sizeof(cl_int), (void *)&i));
	//	checkRet(clEnqueueNDRangeKernel(command_queue, kernels[3], 2, NULL, global_work_size3,/*global_work_size*/ NULL, 0, NULL, /*&events[3] */NULL));
	//}
	//checkRet(clEnqueueReadBuffer(command_queue, arg2, CL_TRUE, 0, _msize(G), G, 0, NULL, NULL));


		isRight = true;
		for (int i = 0; i < row1; i++)
			for (int j = 0; j < col2; j++)
				isRight = C[col2*i + j] == D[col2*i + j] && D[col2*i + j] == E[col2*i + j] && E[col2*i + j] == F[col2*i + j] && F[col2*i + j] == (float)G[col2*i + j];
		//isRight = C[col2*i + j] == D[col2*i + j] == E[col2*i + j] == F[col2*i + j] == G[col2*i + j];
		if (isRight)
			//printf("\nProgram result: Everything works great!\n");
			printf("\nProgram result:  It's all work great!\n");
		else
			printf("\nProgram result: Something went wrong.\n");

	char symbol = NULL;
	do {
		printf("\nDo you want to display result of calcutation? (Y - Yes, N - No)  ");
		rewind(stdin);
		//scanf("%c", &symbol);
		symbol = toupper(getchar());
		if (symbol != 'Y' && symbol != 'N')
			printf("\nI don't know what you mean! Please try again.\n");
	} while (symbol != 'Y' && symbol != 'N');

	if (toupper(symbol) == 'Y')
	{
		printf("\nResult of calcutation:\n");
		printf("\nFirst matrix:\n");
		printMatrix(A, row1, col1);
		printf("\nSecond matrix:\n");
		printMatrix(B, row2, col2);
		printf("\nMultiplied matrix by Host:\n");
		printMatrix(C, row1, col2);
		printf("\nMultiplied matrix by Kernel #0:\n");
		printMatrix(D, row1, col2);
		printf("\nMultiplied matrix by Kernel #1:\n");
		printMatrix(E, row1, col2);
		printf("\nMultiplied matrix by Kernel #2:\n");
		printMatrix(F, row1, col2);
		printf("\nMultiplied matrix by Kernel #3:\n");
		printMatrix(G, row1, col2);
	}

	//do {
	//	printf("\nDo you want to know profiling information? (Y - Yes, N - No)  ");
	//	rewind(stdin);
	//	//scanf("%c", &symbol);
	//	symbol = toupper(getchar());
	//	if (symbol == 'N')
	//		return 0;
	//	if (symbol != 'Y')
	//		printf("\nI don't know what you mean! Please try again.\n");
	//} while (symbol != 'Y');

	//cl_ulong *time = new cl_ulong[2];
	//printf("\nProfiling information:\n");

	//for (int i = 0; i < num_of_kernels; i++)
	//{
	//	checkRet(clGetEventProfilingInfo(events[i], CL_PROFILING_COMMAND_START /*| CL_PROFILING_COMMAND_END*/, sizeof(cl_ulong), &time[0], NULL));
	//	checkRet(clGetEventProfilingInfo(events[i],/* CL_PROFILING_COMMAND_START |*/  CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &time[1], NULL));
	//	//checkRet(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time0), &time0, NULL));
	//	printf("\nTime of execution Kernel #%i is %llu ns or %.9f s", i, time[1] - time[0], (time[1] - time[0])*pow(10, -9)/**10^(-9)*/);
	//}

	/* Finalization */
	for (int i = 0; i < num_of_kernels; i++)
	{
		checkRet(clReleaseKernel(kernels[i]));
		checkRet(clReleaseEvent(events[i]));
	}
	checkRet(clReleaseProgram(program));
	checkRet(clReleaseMemObject(arg0));
	checkRet(clReleaseMemObject(arg1));
	checkRet(clReleaseMemObject(arg2));
	checkRet(clReleaseCommandQueue(command_queue));
	checkRet(clReleaseContext(context));
	free(source_str);
	free(platform_id);
	free(device_id);
	getchar();
	getchar();
	return 0;
}
