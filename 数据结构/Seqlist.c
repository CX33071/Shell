#include "Seqlist.h"
void SeqListCheckCapacity(SeqList* ps) {
    assert(ps);
    if (ps->size == ps->capacity) {
        int newcapacity = ps->capacity * 2;
        SLDataType* newdata =
            (SLDataType*)realloc(ps->data, sizeof(SLDataType) * newcapacity);
        if (newdata == NULL) {
            printf("error");
            return;
        }
        ps->data = newdata;
        ps->capacity = newcapacity;
    }
}
void SeqListInit(SeqList* ps) {
    assert(ps);
    ps->data = (SLDataType*)malloc(sizeof(SLDataType) * N);
    if(ps->data==NULL){
        printf("error");
        return;
    }
    ps->capacity = N;
    ps->size = 0;
}
void SeqListDestroy(SeqList* ps){
    assert(ps);
    free(ps->data);
    ps->data = NULL;
    ps->capacity = 0;
    ps->size = 0;
}
void SeqListPrint(SeqList* ps){
    assert(ps);
    for (int i = 0; i < ps->size;i++){
        printf("%d ", ps->data[i]);
    }
    printf("\n");
}
void SeqListPushBack(SeqList* ps, SLDataType x){
    // assert(ps);
    // SeqListCheckCapacity(ps);
    // ps->data[ps->size] = x;
    // ps->size++;
    SeqListInsert(ps, ps->size, x);
}
void SeqListPushFront(SeqList* ps, SLDataType x){
    // assert(ps);
    // SeqListCheckCapacity(ps);
    // for (int i = ps->size; i > 0; i--) {
    //     ps->data[i] = ps->data[i - 1];
    // }
    //     ps->data[0] = x;
    // ps->size++;
    SeqListInsert(ps, 0, x);
}
void SeqListPopBack(SeqList* ps){
    // assert(ps);
    // if (ps->size == 0) {
    //     printf("error");
    //     return;
    // }
    // ps->size--;
    SeqListErase(ps, ps->size - 1);
}
void SeqListPopFront(SeqList* ps){
    // assert(ps);
    // if (ps->size == 0) {
    //     printf("error");
    //     return;
    // }
    // for (int i = 0; i < ps->size-1; i++) {
    //     ps->data[i] = ps->data[i + 1];
    // }
    // ps->size--;
    SeqListErase(ps, 0);
}
void SeqListInsert(SeqList* ps, int pos, SLDataType x){
    assert(ps);
    if(pos<0||pos>ps->size){
        printf("error");
        return;
    }
    SeqListCheckCapacity(ps);
    for (int i = ps->size; i > pos;i--){
        ps->data[i] = ps->data[i - 1];
    }
    ps->data[pos] = x;
    ps->size++;
}
void SeqListErase(SeqList* ps, int pos){
    assert(ps);
    if(ps->size==0){
        printf("error");
        return;
    }
    if(pos<0||pos>=ps->size){
        printf("error");
        return;
    }
    for (int i = pos+1; i < ps->size;i++){
        ps->data[i - 1] = ps->data[i];
    }
    ps->size--;
}
int SeqListFind(SeqList* ps, SLDataType x){
    assert(ps);
    for (int i = 0; i < ps->size;i++){
        if(ps->data[i]==x){
            return i;
        }
    }
    return -1;
}
void SeqListModify(SeqList* ps, int pos, SLDataType x){
    assert(ps);
    if(pos<0||pos>=ps->size){
        printf("error");
        return;
    }
    ps->data[pos] = x;
}
int SeqListIsEmpty(SeqList* ps){
    assert(ps);
    return ps->size == 0;
}