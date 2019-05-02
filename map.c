#include "map.h"



typedef struct node{
	MapDataElement data_element;
	MapKeyElement key_element;
	struct node* next;
} *Node;


static Node createNode(MapDataElement data_element, MapKeyElement key_element,  Node next) {
	assert(data_element && key_element);
	Node node = malloc(sizeof(*node)); 
	if (node == NULL) return NULL;
	node->data_element = data_element;
	node->key_element = key_element;
	node->next = NULL;
	return node;
}

static void destroyNode(Node node, freeMapDataElements freeDataElement, freeMapKeyElements freeKeyElement) {
	freeKeyElement(node->key_element);
	freeDataElement(node->data_element);
	free(node);
}

static Node copyNode(Node source_node, copyMapDataElements copyDataElement, copyMapKeyElements copyKeyElement) {
	Node node = createNode(copyDataElement(source_node->data_element),
						   copyKeyElement(source_node->key_element), NULL);
	return node;
}

struct Map_t{
	Node head;
	int size;
	MapKeyElement iterator;
	copyMapDataElements copyDataElement;
	copyMapKeyElements copyKeyElement;
	freeMapDataElements freeDataElement;
	freeMapKeyElements freeKeyElement;
	compareMapKeyElements compareKeyElements;
};


Map mapCreate(copyMapDataElements copyDataElement,
			  copyMapKeyElements copyKeyElement,
			  freeMapDataElements freeDataElement,
			  freeMapKeyElements freeKeyElement,
			  compareMapKeyElements compareKeyElements) {
	if (!(copyDataElement && copyKeyElement && freeDataElement && freeKeyElement && compareKeyElements)) {
		return NULL;
	}
	Map map = malloc(sizeof(*map));
	if (map == NULL) return NULL;
	map->head = NULL;
	map->size = 0;
	map->iterator = NULL;
	map->copyDataElement = copyDataElement;
	map->copyKeyElement = copyKeyElement;
	map->freeDataElement = freeDataElement;
	map->freeKeyElement = freeKeyElement;
	map->compareKeyElements = compareKeyElements;
	return map;
}

MapResult mapClear (Map map){
    if (!map) return MAP_NULL_ARGUMENT;
    Node ptr = map->head;
	Node temp = ptr;
    while (ptr) {
        Node to_delete = ptr;
        ptr = ptr->next;
        destroyNode(to_delete, map->freeDataElement, map->freeKeyElement);
    }
	return MAP_SUCCESS;
}
void mapDestroy(Map map) {
    if(!map) return;
    mapClear(map);
	free(map);
}


Map mapCopy(Map map) {
	Map map_cpy = mapCreate(map->copyDataElement,
							map->copyKeyElement,
							map->freeDataElement,
							map->freeKeyElement,
							map->compareKeyElements);
	if (mapGetSize(map) == 0) {
		return map_cpy;
	}
	map_cpy->head = copyNode(map->head, map->copyDataElement, map->copyKeyElement);
	map_cpy->size = map->size;
	Node old_list_ptr = map->head->next;
	Node new_list_ptr = map_cpy->head;
	while (old_list_ptr){
		new_list_ptr->next = copyNode(old_list_ptr, map->copyDataElement, map->copyKeyElement);
		old_list_ptr = old_list_ptr->next;
		new_list_ptr = new_list_ptr->next;
	}
	map_cpy->iterator = NULL;
	return map_cpy;
}

int mapGetSize(Map map) {
	if (!map) return -1;
	return map->size;
}

static Node searchByKey(Map map, MapKeyElement element){
    if (!map || !element) return NULL;
    Node ptr = map->head;
    while(ptr && map->compareKeyElements(ptr->key_element, element) != 0){
        ptr = ptr->next;
    }
	return ptr;
}

MapKeyElement mapGetFirst(Map map){
	if (map == NULL || map->head == NULL) return NULL;
	map->iterator = map->head->key_element;
    return map->head->key_element;
}

MapKeyElement mapGetNext(Map map){
    if (!map || !map->iterator) return NULL;
    Node ptr = searchByKey(map, map->iterator);
    if (!ptr || !ptr->next) return NULL;
	map->iterator = ptr->next->key_element;
    return ptr->next->key_element;
}

bool mapContains (Map map, MapKeyElement element){
	Node ptr = map->head;
	while (ptr != NULL) {
		if (map->compareKeyElements(element, ptr->key_element) == 0) {
			return true;
		}
		ptr = ptr->next;
	}
    return false;
}
MapResult mapPut(Map map, MapKeyElement keyElement, MapDataElement dataElement) {
	if (!(map && keyElement && dataElement)) return MAP_NULL_ARGUMENT;
	MapDataElement dataElement_cpy = map->copyDataElement(dataElement);
	if (dataElement_cpy == NULL) return MAP_OUT_OF_MEMORY;
	if (mapContains(map, keyElement)) {
		Node node = searchByKey(map, keyElement);
		node->data_element = dataElement_cpy;
		return MAP_SUCCESS;
	}
	MapKeyElement keyElement_cpy = map->copyKeyElement(keyElement);
	if (keyElement_cpy == NULL) {
		map->freeDataElement(dataElement_cpy);
		return MAP_OUT_OF_MEMORY;
	}
	Node new_node = createNode(dataElement_cpy, keyElement_cpy, NULL);
	//if map is empty
	if (mapGetSize(map) == 0) {
		map->head = new_node;
		map->size++;
		return MAP_SUCCESS;
	}
	Node current_node = map->head;
	Node previous_node = map->head;
	while (current_node != NULL &&
		map->compareKeyElements(current_node->key_element, new_node->key_element) < 0) {
		previous_node = current_node;
		current_node = current_node->next;
	}
	if (current_node == map->head) {
		new_node->next = current_node;
		map->head = new_node;
	}
	//map reached its final element (or contains only one element)
	else if (current_node == NULL || previous_node == current_node) {
		previous_node->next = new_node;
	}
	else {
		previous_node->next = new_node;
		new_node->next = current_node;
	}
	map->size++;
	return MAP_SUCCESS;
}
MapDataElement mapGet (Map map, MapKeyElement keyElement){
    if (!(map && keyElement)) return NULL;
	MapDataElement element = NULL;
	Node node = (searchByKey(map, keyElement));
	if (node) element = node->data_element;
	return element;
}

/* the function removes a given node from the map.
    if the function was given a NULL argument it will return the proper error.
    if the node does not exist in the map it wil return the proper error.
     otherwise returns MAP_SUCSSES*/
static MapResult nodeRemove (Map map, Node node){
    if(!(map && node)) return MAP_NULL_ARGUMENT;
    Node ptr = map->head;
    if (ptr == node){
        map->head = node->next;
        destroyNode(node, map->freeDataElement, map->freeKeyElement);
        return MAP_SUCCESS;
    }
    while(ptr){
        if (ptr->next == node){
            ptr->next = node->next;
            destroyNode(node, map->freeDataElement, map->freeKeyElement);
            return MAP_SUCCESS;
        }
    }
    if (!ptr) return MAP_ITEM_DOES_NOT_EXIST;
    return MAP_SUCCESS;

}


MapResult mapRemove(Map map, MapKeyElement keyElement){
    if (!(map && keyElement)) return MAP_NULL_ARGUMENT;
    Node node = searchByKey(map,keyElement);
	if (!node) return MAP_ITEM_DOES_NOT_EXIST;
	map->size--;
    return nodeRemove(map, searchByKey(map, keyElement)) ;
}

//================for eurovision use only====================
static void mapBubble(Map map, Node firstNode, Node secondNode) {
	if (!map || !firstNode || !secondNode) return;
	Node previous = map->head;
	if (map->head == firstNode) {
		map->head = secondNode;
		firstNode->next = secondNode->next;
		secondNode->next = firstNode;
		return;
	}
	while (previous->next) {
		if (previous->next == firstNode) break;
		previous = previous->next;
	}
	if (previous->next) {
		previous->next = secondNode;
		firstNode->next = secondNode->next;
		secondNode->next = firstNode;
	}
}
static void printNode(Node node) {
	if (node != NULL) {
		printf("node is: %p\n", node);
		if (node->key_element != NULL)
			printf("	node->keyElement is: %d\n", *(int*)node->key_element);
		else printf("	node->keyElement is null\n");
		if(node->data_element != NULL)
			printf("	node->dataElement is: %d\n", *(int*)node->data_element);
		else printf("	node->dataElement is null\n");

		if (node->next != NULL) {
			printf("	node->next is: %p\n", node->next);
			if (node->next->key_element != NULL)
				printf("	node->next->keyElement is: %d\n", *(int*)node->next->key_element);
			else printf("	node->next->keyElement is null\n");
			if (node->next->data_element != NULL)
				printf("	node->next->dataElement is: %d\n", *(int*)node->next->data_element);
			else printf("	node->next->dataElement is null\n");
		}
		else printf("	node->next is null\n");
	}
	else printf("node is null\n");
}
//sorts the map by key from small to large
void mapSortByKey(Map map) {
	Node node = map->head;
	int iterationSize = mapGetSize(map)-1;
	for (int i = 0; i < mapGetSize(map); i++) {
		node = map->head;
		for (int j = 0; j < iterationSize; j++) {
			if (map->compareKeyElements(node->next->key_element, node->key_element) < 0) {
				mapBubble(map, node, node->next);
			}
			else node = node->next;
		}
		iterationSize--;
	}
}
//sorts the map by data value from large to small
void mapSortByDataForInt(Map map) {
	Node node = map->head;
	int iterationSize = mapGetSize(map) - 1;
	for (int i = 0; i < mapGetSize(map); i++) {
		node = map->head;
		for (int j = 0; j < iterationSize - 1; j++) {
			//printNode(node);
			if (map->compareKeyElements(node->next->data_element, node->data_element) > 0) {
				mapBubble(map, node, node->next);
				/*MAP_FOREACH(int*, iterator, map) {
					printf("iterator is %d\n", *iterator);
				}
				printf("iteration over\n");*/
			}
			else node = node->next;
		}
		iterationSize--;
	}
}

//================end of for eurovision use only====================
