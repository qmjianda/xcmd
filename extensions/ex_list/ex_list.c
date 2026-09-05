#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "xcmd.h"
struct student {
    struct list_head stu_list;
    int ID;
    int math;
};

static int list_test(int argc, char *argv[]) {
    struct student *p;
    struct student stu1;
    struct student stu2;
    struct list_head *pos;
    // 链表的初始化
    INIT_LIST_HEAD(&stu1.stu_list);
    INIT_LIST_HEAD(&stu2.stu_list);
    // 头插法创建stu stu1链表
    for (int i = 0; i < 6; i++) {
        p = (struct student *)malloc(sizeof(struct student));
        p->ID = i;
        p->math = i + 80;
        // 头插法
        list_add(&p->stu_list, &stu1.stu_list);
        // 尾插法
        // list_add_tail(&p->list,&stu.list);
    }

    printf("list_add: \r\n");
    list_for_each(pos, &stu1.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos)->ID, ((struct student *)pos)->math);
    }

    // 尾插法创建stu stu1链表
    for (int i = 0; i < 6; i++) {
        p = (struct student *)malloc(sizeof(struct student));
        p->ID = i;
        p->math = i + 80;
        // 头插法
        // list_add(&p->stu_list,&stu1.stu_list);
        // 尾插法
        list_add_tail(&p->stu_list, &stu2.stu_list);
    }

    printf("list_add_tail: \r\n");
    list_for_each(pos, &stu2.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos)->ID, ((struct student *)pos)->math);
    }
    return 0;
}

static int list_note_del(int argc, char *argv[]) {
    struct student *p;
    struct student stu1;
    struct student stu2;
    struct list_head *pos1;
    // 注意这里的pos2，后面会解释为什么定义为
    struct student *pos2;
    // stu = (struct student*)malloc(sizeof(struct student));
    // 链表的初始化
    INIT_LIST_HEAD(&stu1.stu_list);
    INIT_LIST_HEAD(&stu2.stu_list);
    LIST_HEAD(stu);
    // 头插法创建stu stu1链表
    for (int i = 0; i < 6; i++) {
        p = (struct student *)malloc(sizeof(struct student));
        p->ID = i;
        p->math = i + 80;
        // 头插法
        list_add(&p->stu_list, &stu1.stu_list);
        // 尾插法
        // list_add_tail(&p->list,&stu.list);
    }

    printf("list_add: \r\n");
    list_for_each(pos1, &stu1.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }

    // 删除
    list_for_each_entry(pos2, &stu1.stu_list, stu_list) {
        if (pos2->ID == 4) {
            list_del(&pos2->stu_list);
            break;
        }
    }

    printf("list_del\r\n");
    list_for_each_entry(pos2, &stu1.stu_list, stu_list) {
        printf("ID = %d,math = %d\n", pos2->ID, pos2->math);
    }
    return 0;
}

static int list_note_rep(int argc, char *argv[]) {
    struct student *p;
    struct student stu1;
    struct student stu2;
    struct list_head *pos1;
    struct student *pos2;
    struct student new_obj = {.ID = 100, .math = 100};
    // stu = (struct student*)malloc(sizeof(struct student));
    // 链表的初始化
    INIT_LIST_HEAD(&stu1.stu_list);
    INIT_LIST_HEAD(&stu2.stu_list);
    LIST_HEAD(stu);
    // 头插法创建stu stu1链表
    for (int i = 0; i < 6; i++) {
        p = (struct student *)malloc(sizeof(struct student));
        p->ID = i;
        p->math = i + 80;
        // 头插法
        list_add(&p->stu_list, &stu1.stu_list);
        // 尾插法
        // list_add_tail(&p->list,&stu.list);
    }
    printf("list_add: \r\n");
    list_for_each(pos1, &stu1.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }

    // 替换
    list_for_each_entry(pos2, &stu1.stu_list, stu_list) {
        if (pos2->ID == 4) {
            list_replace(&pos2->stu_list, &new_obj.stu_list);
            break;
        }
    }
    printf("list_replace\r\n");
    list_for_each_entry(pos2, &stu1.stu_list, stu_list) {
        printf("ID = %d,math = %d\n", pos2->ID, pos2->math);
    }
    return 0;
}

static int list_note_del_insert(int argc, char *argv[]) {
    struct student *p;
    struct student stu1;
    struct student stu2;
    struct list_head *pos1;
    struct student *pos2;
    // stu = (struct student*)malloc(sizeof(struct student));
    // 链表的初始化
    INIT_LIST_HEAD(&stu1.stu_list);
    INIT_LIST_HEAD(&stu2.stu_list);
    LIST_HEAD(stu);
    // 头插法创建stu stu1链表
    for (int i = 0; i < 6; i++) {
        p = (struct student *)malloc(sizeof(struct student));
        p->ID = i;
        p->math = i + 80;
        // 头插法
        list_add(&p->stu_list, &stu1.stu_list);
        // 尾插法
        // list_add_tail(&p->list,&stu.list);
    }
    printf("list_add: \r\n");
    list_for_each(pos1, &stu1.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }

    // 移位替换
    list_for_each_entry(pos2, &stu1.stu_list, stu_list) {
        if (pos2->ID == 0) {
            list_move(&pos2->stu_list, &stu1.stu_list);
            break;
        }
    }
    printf("list_move\r\n");
    list_for_each_entry(pos2, &stu1.stu_list, stu_list) {
        printf("ID = %d,math = %d\n", pos2->ID, pos2->math);
    }
    return 0;
}

static int List_to_merge(int argc, char *argv[]) {
    struct student *p;
    struct student *q;
    struct student stu1;
    struct student stu2;
    struct list_head *pos1;
    // stu = (struct student*)malloc(sizeof(struct student));
    // 链表的初始化
    INIT_LIST_HEAD(&stu1.stu_list);
    INIT_LIST_HEAD(&stu2.stu_list);
    LIST_HEAD(stu);
    // 头插法创建stu1 list链表
    for (int i = 0; i < 6; i++) {
        p = (struct student *)malloc(sizeof(struct student));
        p->ID = i;
        p->math = i + 80;
        // 头插法
        list_add(&p->stu_list, &stu1.stu_list);
        // 尾插法
        // list_add_tail(&p->list,&stu.list);
    }
    printf("stu1: \r\n");
    list_for_each(pos1, &stu1.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }
    // 头插法创建stu2 list 链表
    for (int i = 0; i < 3; i++) {
        q = (struct student *)malloc(sizeof(struct student));
        q->ID = i;
        q->math = i + 80;
        // 头插法
        list_add(&q->stu_list, &stu2.stu_list);
        // 尾插法
        // list_add_tail(&p->list,&stu.list);
    }
    printf("stu2: \r\n");
    list_for_each(pos1, &stu2.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }

    // 合并
    list_splice(&stu1.stu_list, &stu2.stu_list);
    printf("list_splice\r\n");
    list_for_each(pos1, &stu2.stu_list) {
        printf("stu2 ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }

    return 0;
}

static int List_traverse(int argc, char *argv[]) {
    struct student *p;
    struct student stu1;
    struct student stu2;
    struct list_head *pos1;
    // stu = (struct student*)malloc(sizeof(struct student));
    // 链表的初始化
    INIT_LIST_HEAD(&stu1.stu_list);
    INIT_LIST_HEAD(&stu2.stu_list);
    LIST_HEAD(stu);
    // 头插法创建stu stu1链表
    for (int i = 0; i < 6; i++) {
        p = (struct student *)malloc(sizeof(struct student));
        p->ID = i;
        p->math = i + 80;
        // 头插法
        list_add(&p->stu_list, &stu1.stu_list);
        // 尾插法
        // list_add_tail(&p->list,&stu.list);
    }
    printf("stu1: \r\n");
    list_for_each(pos1, &stu1.stu_list) {
        printf("ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }

    printf("list_for_each_prev\r\n");
    list_for_each_prev(pos1, &stu1.stu_list) {
        printf("stu2 ID = %d,math = %d\n", ((struct student *)pos1)->ID, ((struct student *)pos1)->math);
    }

    return 0;
}

XCMD_EXPORT_CMD(list, list_test, "list test demo")                             // 链表增加节点
XCMD_EXPORT_CMD(list1, list_note_del, "list note del demo")                    // 链表删除节点
XCMD_EXPORT_CMD(list2, list_note_rep, "list note replace demo")                // 链表替换节点
XCMD_EXPORT_CMD(list3, list_note_del_insert, "list note del and insert demo")  // 链表删除并插入节点
XCMD_EXPORT_CMD(list4, List_to_merge, "List to merge")                         // 链表的合并
XCMD_EXPORT_CMD(list5, List_traverse, "List to merge")                         // 链表的遍历

static xcmd_t cmds[] = {
#ifndef ENABLE_XCMD_EXPORT
    {"list", list_test, "list test demo", NULL},                             // 链表增加节点
    {"list1", list_note_del, "list note del demo", NULL},                    // 链表删除节点
    {"list2", list_note_rep, "list note replace demo", NULL},                // 链表替换节点
    {"list3", list_note_del_insert, "list note del and insert demo", NULL},  // 链表删除并插入节点
    {"list4", List_to_merge, "List to merge", NULL},                         // 链表的合并
    {"list5", List_traverse, "List to merge", NULL},                         // 链表的遍历
#endif
};

void ex_list_init(void) {
    xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
