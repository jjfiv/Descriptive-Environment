/*
Copyright (c) 2013, Charles Jordan <skip@alumni.umass.edu>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/* fdprotos.h
 * Skip Jordan
 *
 * Prototypes for formula distance.
 * chj	 1/25/13	created
 */

/* fd.c */

extern int fd_countvars(struct fd_bvarlist *);
extern struct fdistance *fd_init(struct structure *, struct structure *,
				 int, int, int);
extern struct fd_bvarlist *fd_fdaddlist(struct fdistance *,struct fd_bvarlist *,
					int,int, int, int, int, char **, int *);
extern DdNode *fd_getforms(struct fdistance *, struct structure *);
extern DdNode *fd_getsatforms(struct fdistance *, struct interp *,
			      struct structure *, int *, int *, int, int, int);
extern DdNode *fd_getallsatforms(struct fdistance *, struct interp *,
				 struct structure *,
				 int *vals, int, int);
extern DdNode *fd_realgetsatforms(struct fdistance *, struct interp *,
				  struct structure *, int *vals, int);
extern int fd_countqvars(struct fd_quniv *);
extern int fd_countvars(struct fd_bvarlist *);
extern void output_disting(struct fdistance *, DdNode *, char *, int);
extern struct fd_quniv *fd_initqvars(struct fdistance *);
extern struct fd_bvarlist *fd_initfdvars(struct fdistance *);
extern struct fd_bvarlist *fd_add_predvars(struct fdistance *, int, 
					   struct fd_bvarlist *,
					   struct relation *);
extern struct fd_bvarlist *fd_addpredvar(struct fdistance *,
                                  struct fd_bvarlist *, const int *,
                                  int, struct constant *, char *, int);
extern struct fd_bvarlist *fd_fdaddveqc(struct fdistance *,struct fd_bvarlist *,
				  	int, int, struct constant *);
extern DdNode *fd_qmask_dd(struct fdistance *, int, int *);
extern int fd_geteval(struct interp *, int *vals, struct fd_bvarlist *, int);
extern struct fdistance *fd_init(struct structure *, struct structure *,
				 int, int, int);
extern int fd_issat(struct structure *,
		    struct interp *, int *vals, struct fd_bvarlist *);
extern int fd_issatpred(struct structure *,
			struct interp *, int *vals, struct fd_bvarlist *);
extern void fd_cleanup(struct fdistance *);



