#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pack.h"

int main()
{
	char    buf1[20] = "chenliuliu";
	char    buf2[20] = "dongfeng";
	char    buf3[20] = "xifeng";
	char    buf4[20] = "nanfeng";

	struct mfptp_pack       *root, head, first;
	struct mfptp_pack       *ss, *dd;

	root = &head;
	ss = root;
	head.method = 0x02;
	head.compress = 0x00;
	head.encrypt = 0x00;
	head.version = 0x00;
	head.sub_version = 0x01;
	head.packages = 0x01;
	struct mfptp_frame *root1, head1, first1;
	root1 = &head1;
	head1.size = 10;
	head1.frame_buf = buf1;
	head1.next = &first1;

	first1.size = 8;
	first1.frame_buf = buf2;
	first1.next = NULL;

	head.m_frame = root1;

	/*------------------------*/

	root->next = &first;

	first.method = 0x02;
	first.compress = 0x00;
	first.encrypt = 0x00;
	first.version = 0x00;
	first.sub_version = 0x02;
	first.packages = 0x01;
	struct mfptp_frame *root2, head2, first2;
	root2 = &head2;
	head2.size = 6;
	head2.frame_buf = buf3;
	head2.next = &first2;

	first2.size = 9;
	first2.frame_buf = buf4;
	first2.next = NULL;

	first.next = NULL;

	first.m_frame = root2;

	//  printf("%d\n",root->m_frame->size);
	if (ss == NULL) {
		printf("ssssssssssssssss\n");
	}

	while (ss) {
		printf("%s\n", ss->m_frame->frame_buf);
		ss = ss->next;
	}

	root = mfptp_packages(root);
	int i = 0;
	dd = root;

	for (i = 0; i < 32; i++) {
		if ((i >= 0) && (i <= 11)) {
			printf("%d ", dd->pack_buf[i]);
		} else if ((i >= 22) && (i <= 23)) {
			printf("%d ", dd->pack_buf[i]);
		} else {
			printf("%c ", dd->pack_buf[i]);
		}
	}

	printf("\n");

	return 0;
}

