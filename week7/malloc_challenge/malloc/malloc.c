//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define NUM_BINS 10 // Free List Binの個数

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t {
  size_t size;
  struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t {
  my_metadata_t *free_head;
  my_metadata_t dummy;
} my_heap_t;

typedef struct {
    my_metadata_t *best;
    my_metadata_t *prev;
    int index;
} search_result;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
// my_heap_t my_heap;　// 元の

// FreeLitBinを初期化
// bins[i]: 2^(i+3)以上 2^(i+4)-1以下のサイズのスロットが入っている(i:0~9)
my_heap_t bins[NUM_BINS];

//
// Helper functions (feel free to add/remove/edit!)
//

// 入力のサイズに対してどこのBinに入るのかindexを求める関数
int get_bin_index(size_t size){
  int i; //求めるindex
  for(i = 0; i < NUM_BINS; i++){
    if(pow(2,i+3)<=size && size < pow(2,i+4)){
      break;
    }
  }
  return i;
}

// void my_add_to_free_list(my_metadata_t *metadata) {
//   assert(!metadata->next);
//   metadata->next = my_heap.free_head;
//   my_heap.free_head = metadata;
// }

// Free List Bin用に変更
void my_add_to_free_list(my_metadata_t *metadata) {
  assert(!metadata->next);
  int bin_index = get_bin_index(metadata->size); // 対象のBinのindexを得る
  metadata->next = bins[bin_index].free_head;
  bins[bin_index].free_head = metadata;
}

// void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev) {
//   if (prev) {
//     prev->next = metadata->next;
//   } else {
//     my_heap.free_head = metadata->next;
//   }
//   metadata->next = NULL;
// }

// Free List Bin用に変更
void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev, int bin_index) {
  if (prev) {
    prev->next = metadata->next;
  } else {
    bins[bin_index].free_head = metadata->next;
  }
  metadata->next = NULL;
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
// void my_initialize() {
//   my_heap.free_head = &my_heap.dummy;
//   my_heap.dummy.size = 0;
//   my_heap.dummy.next = NULL;
// }

// Free List Bin用に変更
void my_initialize() {
  // すべてのBinを初期化する
  for(int i = 0; i < NUM_BINS; i++){
    bins[i].free_head = &bins[i].dummy;
    bins[i].dummy.size = 0;
    bins[i].dummy.next = NULL;
  }
}

search_result find_best_bin(my_metadata_t *metadata, my_metadata_t *prev, size_t size, int bin_index){
  my_metadata_t *best_slot = NULL; // 暫定で一番十分な大きさの空き領域のうち最も小さいものを格納する
  size_t min_best_size = SIZE_MAX; // 暫定のサイズの初期値は無限大にする
  my_metadata_t *best_prev = NULL; //best_slotの一つ前を格納する

  // int index = 0;  //debug用変数
  while (metadata) { // metadataがある間繰り返す
    // index++;
    // if (index % 1000 == 0){
    //   printf("【DEBUG】%d\n",index);
    // }
    // 十分な大きさがある かつ 暫定のslotよりサイズが小さいとき更新
    if (size <= metadata->size && metadata->size < min_best_size){
      // printf("【DEBUG】更新\n");
      min_best_size = metadata->size;
      best_slot = metadata;
      best_prev = prev;
    }
    prev = metadata; // 一つ前を保存
    metadata = metadata->next; //次のslotへ
  }
  // 見つからなかったら次の大きさのBinを見る。ただし、NUM_BIN以内のとき
  if ((!best_slot) && bin_index < NUM_BINS-1){
    // printf("【DEGUB】次の大きさへ\n");
    metadata = bins[bin_index+1].free_head;
    return find_best_bin(metadata, NULL, size, bin_index+1);
  }
  // NUM_BINの最大のサイズを見てもない場合はNULLで返すor見つかったときはそのslotをかえす
  else{
    search_result result; //返り値用構造体
    result.best= best_slot;
    result.prev = best_prev;
    result.index = bin_index;
    return result;
  }
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size) {
  // sizeを確認する
  // printf("%zu\n", size);
  // static int call_count = 0;
  //   call_count++;
  //   if (call_count % 100 == 0) {
  //       printf("【MALLOC COUNT】%d 回目 (要求サイズ: %zu)\n", call_count, size);
  //   }

  int bin_index = get_bin_index(size); //入力sizeに対するbinのindexを求める
  // printf("【DEBUG】%d\n",bin_index);
  assert(bin_index >= 0 && bin_index < NUM_BINS); // Binが正しく得られたかチェック

  // my_metadata_t *metadata = my_heap.free_head;
  my_metadata_t *metadata = bins[bin_index].free_head; //対象のBinの先頭を指す
  my_metadata_t *prev = NULL;
  // First-fit: Find the first free slot the object fits.
  // TODO: Update this logic to Best-fit!

  // ↓BestFit&Free List Binへ変更
  search_result result = find_best_bin(metadata, prev, size, bin_index);
  // 返り値を格納
  prev = result.prev;
  metadata = result.best;
  bin_index = result.index;
  // now, metadata points to the first free slot
  // and prev is the previous entry.

  if (!metadata) {
    // There was no free slot available. We need to request a new memory region
    // from the system by calling mmap_from_system().
    //
    //     | metadata | free slot |
    //     ^
    //     metadata
    //     <---------------------->
    //            buffer_size
    size_t buffer_size = 4096;
    my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t);
    metadata->next = NULL;
    // Add the memory region to the free list.
    my_add_to_free_list(metadata);
    // Now, try my_malloc() again. This should succeed.
    // printf("【DEBUG】malloc呼び出し\n");
    return my_malloc(size);
  }

  // |ptr| is the beginning of the allocated object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  void *ptr = metadata + 1;
  size_t remaining_size = metadata->size - size;
  // Remove the free slot from the free list.
  my_remove_from_free_list(metadata, prev,bin_index);

  if (remaining_size > sizeof(my_metadata_t)) {
    // Shrink the metadata for the allocated object
    // to separate the rest of the region corresponding to remaining_size.
    // If the remaining_size is not large enough to make a new metadata,
    // this code path will not be taken and the region will be managed
    // as a part of the allocated object.
    metadata->size = size;
    // Create a new metadata for the remaining free slot.
    //
    // ... | metadata | object | metadata | free slot | ...
    //     ^          ^        ^
    //     metadata   ptr      new_metadata
    //                 <------><---------------------->
    //                   size       remaining size
    my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
    new_metadata->size = remaining_size - sizeof(my_metadata_t);
    new_metadata->next = NULL;
    // Add the remaining free slot to the free list.
    my_add_to_free_list(new_metadata);
  }

  //DEBUG
  if (ptr == NULL) {
        printf("【警報】my_malloc が NULL を返しました！\n");
    }
    return ptr;
  return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr) {
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  // Add the free slot to the free list.
  my_add_to_free_list(metadata);
}

// This is called at the end of each challenge.
void my_finalize() {
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test() {
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
