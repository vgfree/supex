/*关闭父进程所有打开的文件描述符， pfd:此描述符不关闭*/
static bool _close_all_fd(int pfd)
{
	int             retval = 0;
	int             rewind = 0;
	int             fd = 0;
	DIR             *dir = NULL;
	struct dirent   *entry, _entry;

	if (unlikely(!(dir = opendir("/dev/fd")))) {
		return false;
	}

	while (1) {
		retval = readdir_r(dir, &_entry, &entry);

		if (unlikely(retval != 0)) {
			closedir(dir);
			return false;
		}

		if (entry == NULL) {
			if (!rewind) {
				break;
			}

			rewinddir(dir);
			rewind = 0;
			continue;
		}

		if (entry->d_name[0] == '.') {
			continue;
		}

		fd = atoi(entry->d_name);

		if (unlikely((dirfd(dir) == fd) || (fd == 1) || (fd == 0) || (fd == 2) || (fd == pfd))) {
			continue;
		}

		close(fd);
		rewind = 1;
	}

	closedir(dir);
	return true;
}

/* 非递归的快速排序 */
static bool _quick_sort(int array[], int n)
{
	struct stack
	{
		int     start;	/* 数组中第一个元素下标 */
		int     end;	/* 数组中最后一个元素下标*/
	};

	int             index = 0;			/* 栈的下标 */
	int             size = sizeof(struct stack);	/* struct stack结构体大小 */
	int             i = 0, j = 0, key = 0;
	int             left = 0, right = 0;
	struct stack    *stack = NULL;
	char            *memory = calloc(n, size);

	if (memory) {
		stack = (struct stack *)&(memory[index * size]);
		stack->start = 0;
		stack->end = n - 1;

		while (index > -1) {
			i = left = stack->start;	/* 数组从左开始数据的下标 */
			j = right = stack->end;		/* 数组从右开始数据的下标 */
			key = array[left];
			index--;

			while (i < j) {
				while ((i < j) && (key <= array[j])) {
					j--;
				}

				if (i < j) {
					array[i] = array[i] ^ array[j];
					array[j] = array[i] ^ array[j];
					array[i] = array[i] ^ array[j];
					i++;
				}

				while ((i < j) && (key >= array[i])) {
					i++;
				}

				if (i < j) {
					array[i] = array[i] ^ array[j];
					array[j] = array[i] ^ array[j];
					array[i] = array[i] ^ array[j];
					j--;
				}
			}

			if (left < i - 1) {
				index++;
				stack = (struct stack *)&memory[index * size];
				stack->start = left;
				stack->end = i - 1;
			}

			if (right > i + 1) {
				index++;
				stack = (struct stack *)&memory[index * size];
				stack->start = i + 1;
				stack->end = right;
			}

			stack = (struct stack *)&memory[index * size];
		}

		free(memory);
		return true;
	}

	return false;
}

