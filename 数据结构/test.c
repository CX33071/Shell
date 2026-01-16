#include "Seqlist.h"
int main() {
    SeqList sl;
    SeqListInit(&sl);
    SeqListPushBack(&sl, 1);
    SeqListPushBack(&sl, 2);
    SeqListPushBack(&sl, 3);
    printf("尾插后：");
    SeqListPrint(&sl);  
    SeqListPushFront(&sl, 0);
    printf("头插后：");
    SeqListPrint(&sl);  
    SeqListInsert(&sl, 2, 0);
    printf("下标2插入0：");
    SeqListPrint(&sl); 
    int pos = SeqListFind(&sl, 0);
    printf("元素0的下标：%d\n", pos);  
    SeqListModify(&sl, pos, 100);
    printf("修改后：");
    SeqListPrint(&sl);  
    SeqListErase(&sl, pos);
    printf("删除下标%d后：", pos);
    SeqListPrint(&sl);  
    SeqListPopBack(&sl);
    SeqListPopFront(&sl);
    printf("尾删+头删后：");
    SeqListPrint(&sl); 
    SeqListDestroy(&sl);
    return 0;
}
