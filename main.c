/* Include some files */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "map.h"
#include "test_utilities.h"


//The following block contains compare/copy/free function for Integers
static MapKeyElement copyInt(MapKeyElement e) {
    int *newInt = malloc(sizeof(int));
    if (newInt == NULL) return NULL;
    *newInt = *(int *) e;
    return newInt;
}

static void freeInt(MapKeyElement e) {
    free(e);
}

static int compareInt(MapKeyElement a, MapKeyElement b) {
    return *(int *) a - *(int *) b;
}


//The tests block
static int createDestroyTest(int *tests_passed) {
    _print_mode_name("Testing Create&Destroy functions");
    int test_number = 1;
    _print_test_number(test_number, __LINE__);
    Map map = mapCreate(NULL, copyInt, freeInt, freeInt, compareInt);
    test(map != NULL, __LINE__, &test_number, "mapCreate doesn't return NULL on NULL CopyDataElement", tests_passed);
    map = mapCreate(copyInt, NULL, freeInt, freeInt, compareInt);
    test(map != NULL, __LINE__, &test_number, "mapCreate doesn't return NULL on NULL CopyKeyElement", tests_passed);
    map = mapCreate(copyInt, copyInt, NULL, freeInt, compareInt);
    test(map != NULL, __LINE__, &test_number, "mapCreate doesn't return NULL on NULL FreeDataElement", tests_passed);
    map = mapCreate(copyInt, copyInt, freeInt, NULL, compareInt);
    test(map != NULL, __LINE__, &test_number, "mapCreate doesn't return NULL on NULL FreeKeyElement", tests_passed);
    map = mapCreate(copyInt, copyInt, freeInt, freeInt, NULL);
    test(map != NULL, __LINE__, &test_number, "mapCreate doesn't return NULL on NULL CompareKeyElement", tests_passed);
    map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    test(map == NULL, __LINE__, &test_number, "mapCreate returns NULL on Valid input", tests_passed);
    mapDestroy(NULL); // A silent test which check if the function doesn't crash on NULL input.
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    return test_number;
}

static int mapPutTest(int *tests_passed) {
    _print_mode_name("Testing mapPut function");
    int test_number = 1;
    int a[6] = {0, 1, 2, 3, 4, 5};
    Map map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    MapResult mr;
    _print_test_number(test_number, __LINE__);
    mr = mapPut(NULL, &a[0], &a[1]);
    test( mr != MAP_NULL_ARGUMENT , __LINE__, &test_number, "mapPut doesn't return MAP_NULL_ARGUMENT on NULL map input", tests_passed);
    mr = mapPut(map, NULL, &a[1]);
    test( mr == MAP_SUCCESS , __LINE__, &test_number, "mapPut return MAP_SUCCESS on NULL key input", tests_passed);
    mr = mapPut(map, &a[0], NULL);
    test( mr == MAP_SUCCESS , __LINE__, &test_number, "mapPut return MAP_SUCCESS on NULL data input", tests_passed);
    mr = mapPut(map, &a[0], &a[1]);
    test( mr != MAP_SUCCESS , __LINE__, &test_number, "mapPut doesn't return MAP_SUCCESS on valid data input", tests_passed);
    mapPut(map, &a[2], &a[3]);
    mapGetFirst(map);
    mapPut(map, &a[4], &a[5]);                         //Implementation dependent
    test( mapGetNext(map) != NULL, __LINE__, &test_number, "mapPut doesn't set iterator be NULL after insertion", tests_passed);
    bool ordered = true;
    int k = 0;
    MAP_FOREACH(int*, i, map) {
        if((a[k] != *i)) {
            ordered = false;
            break;
        }
        k+=2;
    }//                                         Inserting keys order: low->mid->high
    test( !ordered , __LINE__, &test_number, "mapPut doesn't order the keys correctly, Insertion from low to high", tests_passed);
    mapDestroy(map);
    map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    mapPut(map, &a[4], &a[5]);
    mapPut(map, &a[2], &a[3]);
    mapPut(map, &a[0], &a[1]);
    ordered = true;
    k = 0;
    MAP_FOREACH(int*, i, map) {
        if((a[k] != *i)) {
            ordered = false;
            break;
        }
        k+=2;
    }//                                         Inserting keys order: high->mid->low
    test( !ordered , __LINE__, &test_number, "mapPut doesn't order the keys correctly, Insertion from high to low", tests_passed);
    mapDestroy(map);
    map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    mapPut(map, &a[2], &a[3]);
    mapPut(map, &a[4], &a[5]);
    mapPut(map, &a[0], &a[1]);
    k = 0;
    ordered = true;
    MAP_FOREACH(int*, i, map) {
        if((a[k] != *i)) {
            ordered = false;
            break;
        }
        k+=2;
    }//                                            Inserting keys order: small->high->middle(in between them)
    test( !ordered , __LINE__, &test_number, "mapPut doesn't order the keys correctly, Insertion in the middle", tests_passed);
    mr = mapPut(map, &a[2], &a[1]);
    test( mr != MAP_SUCCESS , __LINE__, &test_number, "mapPut doesn't allow to Re-write data on existing key", tests_passed);
    MapDataElement data = mapGet(map, &a[2]);
    test( *(int *)data != a[1] , __LINE__, &test_number, "mapPut doesn't rewrite the data on existing key", tests_passed);
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    return test_number;
}

static int mapGetSizeTest(int *tests_passed) {
    _print_mode_name("Testing mapGetSize function");
    int test_number = 1;
    _print_test_number(test_number, __LINE__);
    test( mapGetSize(NULL) != -1 , __LINE__, &test_number, "mapGetSize doesn't return -1 on NULL map Input", tests_passed);
    Map map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    test( mapGetSize(map) != 0 , __LINE__, &test_number, "mapGetSize doesn't return 0 on empty map Input", tests_passed);
    int a[2] = {0, 1};
    mapPut(map, &a[0], &a[1]);
    test( mapGetSize(map) != 1 , __LINE__, &test_number, "mapGetSize doesn't return right value", tests_passed);
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    return test_number;
}

static int mapCopyTest(int *tests_passed) {
    _print_mode_name("Testing mapCopyTest function");
    int test_number = 1;
    _print_test_number(test_number, __LINE__);
    int a[6] = {0, 1, 2, 3, 4, 5};
    Map map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    mapPut(map, &a[0], &a[1]);
    mapPut(map, &a[2], &a[3]);
    mapPut(map, &a[4], &a[5]);
    Map map_copy = mapCopy(map);                          //Assuming mapGetSize works correctly.
    test(mapGetSize(map) != mapGetSize(map_copy), __LINE__, &test_number, "mapCopy doesn't copy every pair of  elements", tests_passed);
    test(mapCopy(NULL) != NULL ,__LINE__, &test_number, "mapCopy doesn't return NULL on NULL input", tests_passed);
    int k = 0;
    bool ordered = true;
    MAP_FOREACH(int*, i, map_copy) {
        if((a[k] != *i)) {
            ordered = false;
            break;
        }
        k+=2;
    }
    test( !ordered , __LINE__, &test_number, "mapCopy doesn't keep the order of the keys", tests_passed);
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    mapDestroy(map_copy);
    return test_number;
}

static int mapContainsTest(int *tests_passed) {
    _print_mode_name("Testing mapContains function");
    int test_number = 1;
    _print_test_number(test_number, __LINE__);
    int a[6] = {0, 1, 2, 3, 4, 5};
    Map map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    test( mapContains(NULL, &a[0]) != false,__LINE__, &test_number, "mapContain doesn't return false on NULL map input", tests_passed);
    test( mapContains(map, NULL) != false,__LINE__, &test_number, "mapContain doesn't return false on NULL element input", tests_passed);
    test( mapContains(map, &a[0]) != false,__LINE__, &test_number, "mapContain doesn't return false on key which is not in map", tests_passed);
    mapPut(map, &a[0], &a[1]);
    test( !mapContains(map, &a[0]) ,__LINE__, &test_number, "mapCopy doesn't return true on existing element", tests_passed);
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    return test_number;
}


static int mapRemoveTest(int *tests_passed) {
    _print_mode_name("Testing mapRemove function");
    int test_number = 1;
    _print_test_number(test_number, __LINE__);
    int a[6] = {0, 1, 2, 3, 4, 5};
    Map map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    mapPut(map, &a[0], &a[1]);
    test( mapRemove(NULL, &a[0]) != MAP_NULL_ARGUMENT,__LINE__, &test_number, "mapRemove doesn't return MAP_NULL_ARGUMENT on NULL map input", tests_passed);
    test( mapRemove(map, NULL) != MAP_NULL_ARGUMENT,__LINE__, &test_number, "mapRemove doesn't return MAP_NULL_ARGUMENT on NULL key input", tests_passed);
    test( mapRemove(map, &a[1]) != MAP_ITEM_DOES_NOT_EXIST, __LINE__, &test_number, "mapRemove doesn't return MAP_ITEM_DOES_NOT_EXIST on key not which is not in map", tests_passed);
    test( mapRemove(map, &a[0]) != MAP_SUCCESS, __LINE__, &test_number, "mapRemove doesn't return MAP_SUCCESS after removal", tests_passed);
    test( mapContains(map, &a[0]) == true, __LINE__, &test_number, "mapRemove doesn't remove key from the dictionary", tests_passed);
    //                                                  Implementation dependent
    test( mapGetNext(map) != NULL, __LINE__, &test_number, "mapRemove doesn't set the iterator to be NULL after remove.", tests_passed);
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    return test_number;
}

static int mapClearTest(int *tests_passed) {
    _print_mode_name("Testing mapClear function");
    int test_number = 1;
    _print_test_number(test_number, __LINE__);
    int a[6] = {0, 1, 2, 3, 4, 5};
    Map map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    mapPut(map, &a[0], &a[1]);
    test( mapClear(NULL) != MAP_NULL_ARGUMENT ,__LINE__, &test_number, "mapClear doesn't return MAP_NULL_ARGUMENT on NULL map input", tests_passed);
    test( mapClear(map) != MAP_SUCCESS , __LINE__, &test_number, "mapClear doesn't return MAP_SUCCESS after clear", tests_passed);
    test( mapContains(map, &a[0]) == true, __LINE__, &test_number, "mapClear doesn't remove elements from the map", tests_passed);
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    return test_number;
}

static int mapGetTest(int *tests_passed) {
    _print_mode_name("Testing mapGetFirst/mapGetNext function:");
    int test_number = 1;
    _print_test_number(test_number, __LINE__);
    test( mapGetNext(NULL) != NULL, __LINE__, &test_number, "mapGetNext doesn't return NULL on NULL map input.", tests_passed);
    test( mapGetFirst(NULL) != NULL, __LINE__, &test_number, "mapGetFirst doesn't return NULL on NULL map input.", tests_passed);
    int a[6] = {0, 1, 2, 3, 4, 5};
    Map map = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
    test( mapGetFirst(map) != NULL, __LINE__, &test_number, "mapGetFirst doesn't return NULL on empty map input", tests_passed);
    mapPut(map, &a[0], &a[1]);
    test( compareInt(mapGetFirst(map), &a[0]) != 0, __LINE__, &test_number, "mapGetFirst doesn't return right element", tests_passed);
    MapKeyElement key = mapGetNext(map);
    test( key != NULL, __LINE__, &test_number, "mapGetNext doesn't return NULL at the end of the map", tests_passed);
    mapPut(map, &a[2], &a[3]);
    mapGetFirst(map);
    key = mapGetNext(map);
    test( compareInt(key, &a[2]) != 0, __LINE__, &test_number, "mapGetNext doesn't return right element", tests_passed);
    _print_test_success(test_number);
    *tests_passed += 1;
    mapDestroy(map);
    return test_number;
}

int main() {
    printf("\nWelcome to the homework 3 map_module tests, written by Vova Parakhin.\n\n---Passing those tests won't "
           "guarantee you a good grade---\nBut they might get you close to one "
           ",make some tests yourself to be sure.\n\n");
    printf("You can change w/e you want in the file itself but make sure \nto contact me if you want to upload");
    printf(" \'upgraded version\' of the file\n");
    printf("\nPress any button to start the tests:. \n\n");
    getchar();
    int tests_passed = 0;
    int tests_number = 0;
    tests_number += createDestroyTest(&tests_passed);
    tests_number += mapPutTest(&tests_passed);
    tests_number += mapGetSizeTest(&tests_passed);
    tests_number += mapCopyTest(&tests_passed);
    tests_number += mapContainsTest(&tests_passed);
    tests_number += mapRemoveTest(&tests_passed);
    tests_number += mapClearTest(&tests_passed);
    tests_number += mapGetTest(&tests_passed);
    print_grade(tests_number, tests_passed);
    return 0;
}
