#ifndef LISTS_H
#define LISTS_H

struct node
{
	struct node *succ;
	struct node *pred;
	char        *name;
	void        *data;
};


struct list
{
	struct node *head;
	struct node *tail;
	struct node *tailpred;
	long         cnt;
};

void list_init( struct list *l );
void list_saveitem( struct list *l, char *name, void *data );
struct node *list_getnext( struct list *l, struct node *n, void **data_ptr );
struct node *list_findname( struct list *l, char *name );
void list_insert( struct list *l, struct node *n, struct node *npred );
void list_remove( struct list *l, struct node *n );
void list_addhead( struct list *l, struct node *n );
void list_addtail( struct list *l, struct node *n );
struct node *list_remhead( struct list *l );
struct node *list_remtail( struct list *l );

#endif /* LISTS_H */
