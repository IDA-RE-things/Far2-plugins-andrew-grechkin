namespace Base {

//template<class InputIterator, class OutputIterator>
//void PermutationWithRepetition(size_t K, InputIterator start, InputIterator end, OutputIterator out)
//{
//	OperationsContainer pIn(start, end), pOut(K);
//	OperationsContainer::size_type in_size = pIn.size();
//
//	K--;
//	int *stack = new int[K * 2],  // стек псевдорекурсии, глубина рекурсии K - 1
//		*pTop = stack,            // вершина стека
//		k = 0,                    // переменные цикла
//		n = 0,
//		j = 0;
//	for (;;)
//	{
//		while(n < in_size)
//		{
//			pOut[k] = pIn[n++];
//			if (k == K) {
//				++j;
//				*out = pOut;
//								printOperations(j, pOut);
//				//				printf("%02d. %s\n", ++j, pOut);
//			} else {
//				if (n < in_size)
//				{
//					*pTop++ = k;          // сохраняем k и n в стеке
//					*pTop++ = n;
//				}
//				k++;                    // псевдорекурсивный вызов
//				n = 0;
//			}
//		}
//		if (pTop == stack)          // стек пуст, конец цикла
//			break;
//
//		n = *(--pTop);              // выталкиваем k и n из стека
//		k = *(--pTop);
//	}
//	delete[] stack;
//}
//
//Number count_poliz(Operation * op[], size_t size) {
//	Number ret = 0;
//	NumbersContainer stack(size);
//	size_t m = 0;
//	for (size_t i = 0; i < size; ++i)
//	{
//		if (op[i]->get_type() == AdjustNumber) {
//			stack[m] = op[i]->get_value();
//			m++;
//			continue;
//		}
//
//		if (m < 2)
//			return -1;
//
//		switch (op[i]->get_type())
//		{
//			case Addition:
//			{
//				ret = stack[m - 2] + stack[m - 1];
//				break;
//			}
//			case Subtraction:
//			{
//				if (stack[m - 2] < stack[m - 1])
//					return -1;
//				ret = stack[m - 2] - stack[m - 1];
//				break;
//			}
//			case Multiplication:
//			{
//				ret = stack[m - 2] * stack[m - 1];
//				break;
//			}
//			case Division:
//			{
//				if (stack[m - 2] < stack[m - 1])
//					return -1;
//				ret = stack[m - 2] / stack[m - 1];
//				break;
//			}
//			default:
//				break;
//		}
//
//		stack[m - 2] = ret;
//		m--;
//	}
//	return ret;
//}

}
