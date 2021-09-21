
__kernel void MatrixMultiply0( __global float* a, __global float* b, __global float* c, int Wa, int Wb) {
	int row = get_global_id(0);
	for (int col = 0; col < Wb; col++) 
	{
		float sum= 0.0f;
		for (int i = 0; i < Wa; i++) {
			sum +=a[row*Wa + i] * b[i*Wb + col];
		}
		c[row*Wb + col] = sum;
	}
	//printf("(%i,%i) or (%i) : %f\n",row, col, row*Wb + col, sum);
}

__kernel void MatrixMultiply1(__global float* a, __global float* b, __global float* c, int Wa, int Wb, int La) {
	int col = get_global_id(0);

	for (int row = 0; row < La; row++) 
	{
		float sum = 0.0f;
		for (int i = 0; i < Wa; i++) {
			sum +=a[row*Wa + i] * b[i*Wb + col];
		} 
		c[row*Wb + col] = sum;
	}
}

__kernel void MatrixMultiply2( __global float* a, __global float* b, __global float* c, int Wa, int Wb) {
	int row = get_global_id(0);
	int col = get_global_id(1);
	float sum = 0.0f;
	for (int i = 0; i < Wa; i++) {
		sum += a[row*Wa + i] * b[i*Wb + col];
	}
	c[row*Wb + col] = sum;
}


inline void AtomicAdd(volatile __local float *source, const float operand)
{	union {
		unsigned int intVal;
		float floatVal;
	} newVal;
	union {
		unsigned int intVal;
		float floatVal;
	} prevVal;
	do {
		prevVal.floatVal = *source;
		newVal.floatVal = prevVal.floatVal + operand;
	} while (atomic_cmpxchg((volatile __local unsigned int*)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);
}

__kernel void MatrixMultiply3(__global float* a, __global float* b, __global float* c, int Wa, int Wb) {

	//int Wa = get_global_size(2);
	int row = get_global_id(0);
	int col = get_global_id(1);
	int hmm = get_global_id(2);
	local float *value;
	*value = 0.0f;
	float res = a[row*Wa + hmm] * b[hmm*Wb + col];

	///*if (row == 0 && col == 0) */printf("(%i, %i, %i)  %f +%.4f\n", row, col, hmm,*value,  res);
	AtomicAdd(value, res);
	///if (row == 0 && col == 0 && hmm == 0) printf("(%i, %i, %i) === %f\n", row, col, hmm, *value);
	barrier(CLK_LOCAL_MEM_FENCE);
	//work_group_barrier(CLK_LOCAL_MEM_FENCE);  //Nvidia могет, а Intel нет (странно)
	//if (/*row == 0 && col == 0 && */hmm == 0)  printf("====================================\n %f + %f =", c[row*Wb + col], *value);
	c[row*Wb + col] += *value;
	//c[row*Wb + col] = *value;
	//if (/*row == 0 && col == 0 &&*/ hmm == 0) printf(" %f\n", c[row*Wb + col]);

	/*хз зачем это ваще*/
	value = 0;
	local double val;
	val = val + 1;
}

//Old kernel #3:
//__kernel void MatrixMultiply3( __global float* a, __global float* b, __global float* c, int Wa, int Wb/*, int hmm*/) {
//	int row = get_global_id(0);
//	int col = get_global_id(1);
//	int hmm = get_global_id(2); /*mb ray*/
//	c[row*Wb + col] += a[row*Wa + hmm] * b[hmm*Wb + col];
//}

