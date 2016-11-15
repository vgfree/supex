/**
 * @file   fcontext.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Oct 10 15:18:43 2012
 *
 * @brief  Header file for fast context.
 *
 */

/*
 * Copyright (c) 2012 Reginald Lips
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions: The above copyright notice and this
 * permission notice shall be included in all copies or substantial
 * portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FCONTEXT_H_
#define FCONTEXT_H_

enum {
	FREG_R12 = 0,
	FREG_R13,
	FREG_R14,
	FREG_R15,
	FREG_RDI,
	FREG_RBP,
	FREG_RBX,
	FREG_RSP,
	FREG_RIP,
	NB_FREG
};

typedef struct s_fstack {
	void *sp;
	size_t size;
} t_fstack;

typedef struct s_fcontext {
	long int reg[NB_FREG];
	struct s_fcontext *link;
	t_fstack stack;
} t_fcontext;

void fcontext(t_fcontext *ctx, void (*func)(void *ptr), void *arg);
void fcontext_jump(void);
int fcontext_swap(t_fcontext *octx, t_fcontext *nctx);

#endif /* !FCONTEXT_H_ */
