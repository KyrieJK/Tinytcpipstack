//
// Created by KyrieJK on 2019/4/23.
//
/**
 * 双向循环链表数据结构
 * 用来存储设备、数据包等有关结构在通信协议栈中的抽象实体
 */

#ifndef TINYTCPIPSTACK_LIST_H
#define TINYTCPIPSTACK_LIST_H

//开发过程中发现NULL not declared，由于通过宏定义NULL。NULL宏定义包含在<stddef.h>头文件中
#ifndef NULL
#define NULL ((void*)0)
#endif

struct list_head{
    struct list_head *prev,*next;
};

static inline void list_init(struct list_head *head){
    head->prev = head->next = head;
}

static inline void __list_add(struct list_head *new_list, struct list_head *prev, struct list_head *next){
    new_list->next = next;
    new_list->prev = prev;
    next->prev = new_list;
    prev->next = new_list;
}

static inline void list_add(struct list_head *new_list, struct list_head *head){
    __list_add(new_list,head,head->next);
}

static inline void list_add_tail(struct list_head *new_list, struct list_head *head){
    __list_add(new_list,head->prev,head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next){
    prev->next = next;
    next->prev = prev;
}

/**
 * 从双向链表中去除del_list元素，并清除del_list结构体中的内容
 * @param del_list
 */
static inline void list_del(struct list_head *del_list){
    __list_del(del_list->prev,del_list->next);
    del_list->prev = NULL;
    del_list->next = NULL;
}



/**
 * 从双向循环链表中去除del_list元素后，并不清空del_list结构体中的元素，而是使其内部成员指针*prev,*next指向自己
 * @param del_list
 */
static inline void list_del_init(struct list_head *del_list){
    __list_del(del_list->prev,del_list->next);
    del_list->prev = del_list;
    del_list->next = del_list;
}

//根据结构体中的成员内存地址以及成员相对于结构体首地址的偏移量，计算得出结构体首地址
#define container_of(ptr,type,member)\
    ((type*)((char*)(ptr)-(size_t)&((type*)0)->member))

//本项目定义的双向循环链表头节点为哑元节点
#define LIST_HEAD(name)\
    struct list_head name = {&name,&name};

#define list_empty(head) ((head)==(head)->next)

#define list_entry(ptr,type,member) container_of(ptr,type,member)
#define list_first_entry(head,type,member)\
    list_entry((head)->next,type,member)
#define list_last_entry(head,type,member)\
    list_entry((head)->prev,type,member)

#define list_for_each_entry(entry,head,member)\
    for(entry = list_first_entry(head,typeof(*entry),member);\
        &entry->member != (head);\
        entry=list_first_entry(&entry->member,typeof(*entry),member))

#define list_for_each_entry_continue(entry,head,member)\
    for(;&entry->member!=(head);\
        entry = list_first_entry(&entry->member,typeof(*entry),member))

#define list_for_each_entry_safe(entry,next,head,member)\
    for(entry=list_first_entry(head,typeof(*entry),member),\
        next = list_first_entry(&entry->member,typeof(*entry),member),\
        &entry->member != (head);\
        entry=next,next = list_first_entry(&next->member,typeof(*entry),member))

#define list_for_each_entry_safe_continue(entry,next,head,member)\
    for(entry = list_first_entry(&entry->member,typeof(*entry),member);\
        &entry->member!=(head);\
        entry=next,next = list_first_entry(&next->member,typeof(*entry),member))

#define list_for_each_entry_reverse(entry,head,member)\
    for(entry = list_last_entry(head,typeof(*entry),member);\
        &entry->member != (head);\
        entry = list_last_entry(&entry->member,typeof(*entry),member))

/**
 * 哈希链表
 * 非循环链表
 * Hash list区分头节点和数据节点
 */

//哈希链表头节点
struct hlist_head{
    struct hlist_node *first;
};
//哈希链表数据节点
struct hlist_node{
    struct hlist_node *next;
    struct hlist_node **pprev;
};

static inline int hlist_unhashed(struct hlist_node *node){
    return !node->pprev;
}

static inline int hlist_empty(struct hlist_head *head){
    return !head->first;
}

static inline void hlist_head_init(struct hlist_head *head){
    head->first = NULL;
}

static inline void hlist_node_init(struct hlist_node *node){
    node->next = NULL;
    node->pprev = NULL;
}

static inline void __hlist_del(struct hlist_node *n){
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;
    *pprev=next;
    if (n->next != NULL) {
        n->next->pprev = n->pprev;
    }
}

static inline void hlist_del(struct hlist_node *n){
    __hlist_del(n);
    n->next = NULL;
    n->pprev = NULL;
}

/**
 * 在头节点之后添加数据节点
 * @param n
 * @param head
 */
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *head){
    n->next = head->first;
    n->pprev = &head->first;
    if(head->first!=NULL){
        head->first->pprev = &n->next;
    }
    head->first = n;
}

static inline void hlist_add_before(struct hlist_node *n, struct hlist_node *next){
    n->next = next;
    n->pprev = next->pprev;
    *(next->pprev) = n;
    next->pprev = &n->next;
}

/**
 * 将数据节点n添加至next后
 * @param n
 * @param next
 */
static inline void hlist_add_after(struct hlist_node *n, struct hlist_node *next){
    n -> next = next->next;
    n->pprev = &next->next;
    if(next->next!=NULL){
        next->next->pprev = &n->next;
    }
    next->next = n;
}

#define hlist_entry(ptr,type,member) list_entry(ptr,type,member)

#define hlist_for_each_entry(entry,node,head,member)\
    for(node=head->first;\
    node && (entry = hlist_entry(node,typeof(*entry),member));\
    node = node->next)

#define hlist_for_each_entry_ifnull(entry,head,member)\
    for(entry = (head->first ? hlist_entry(head->first,typeof(*entry),member):NULL);\
        entry;\
        entry = (entry->member.next) ? hlist_entry(entry->member.next,typeof(*entry),member) : NULL)

#endif //TINYTCPIPSTACK_LIST_H
