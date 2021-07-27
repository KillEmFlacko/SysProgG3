/*! \file lib.c
 * \brief Template library file.
 *	
 *	This is a template code file for a static library.
 */

#include <stdlib.h>
#include <limits.h>
#include "lib/lib.h"

/*! \brief Add two integers.
 *
 * \param a First integer.
 * \param b Second integer.
 * \return The sum.
 */
int add(int a, int b){
	return a+b;
}

/*! \brief Find max value in array
 *
 * \param array array in which we want to find the maximum
 * \param len array length
 * \param max_index pointer that will contain the index of the maximum value
 * \return the maximum value in the array
 */
int array_max(int* array, int len, int *max_index)
{
	int i_max;
	int max = INT_MIN;

	int i;
	for(i = 0; i < len ; i++)
	{
		if(array[i] > max)
		{
			i_max = i;
			max = array[i];
		}
	}

	if(max_index != NULL) *max_index = i_max;
	return max;
}
