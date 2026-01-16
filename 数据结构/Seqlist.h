#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define N 10
// typedef int SLDataType;
// typedef struct SeqList{
//     SLDataType arr[N];
//     int size;
// } SeqList;
typedef int SLDataType;
typedef struct SeqList {
    SLDataType* data; 
    int size;         
    int capacity;     
} SeqList;
void SeqListCheckCapacity(SeqList* ps);
void SeqListInit(SeqList* ps);
void SeqListDestroy(SeqList* ps);
void SeqListPrint(SeqList* ps);
void SeqListPushBack(SeqList* ps, SLDataType x);
void SeqListPushFront(SeqList* ps, SLDataType x);
void SeqListPopBack(SeqList* ps);
void SeqListPopFront(SeqList* ps);
void SeqListInsert(SeqList* ps, int pos, SLDataType x);
void SeqListErase(SeqList* ps, int pos);
int SeqListFind(SeqList* ps, SLDataType x);
void SeqListModify(SeqList* ps, int pos, SLDataType x);
int SeqListIsEmpty(SeqList* ps);