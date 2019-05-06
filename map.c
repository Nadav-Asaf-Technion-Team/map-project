#include "map.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
	MapDataElement data_element;
	MapKeyElement key_element;
	struct node* next;
} *Node;

//creates a sigle node for the map
static Node createNode(MapDataElement data_element, MapKeyElement key_element,
	Node next) {
	assert(data_element && key_element);
	Node node = malloc(sizeof(*node));
	if (node == NULL) return NULL;
	node->data_element = data_element;
	node->key_element = key_element;
	node->next = NULL;
	return node;
}
//destroys a single node
static void destroyNode(Node node, freeMapDataElements freeDataElement,
	freeMapKeyElements freeKeyElement) {
	freeKeyElement(node->key_element);
	freeDataElement(node->data_element);
	free(node);
	printf("check\n");
	printf("2");
}
//copies a sigle node
static Node copyNode(Node source_node, copyMapDataElements copyDataElement,
	copyMapKeyElements copyKeyElement) {
	Node node = createNode(copyDataElement(source_node->data_element),
		copyKeyElement(source_node->key_element), NULL);
	return node;
}

struct Map_t {
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
	if (!(copyDataElement && copyKeyElement && freeDataElement
		&& freeKeyElement && compareKeyElements)) {
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

MapResult mapClear(Map map) {
	if (!map) return MAP_NULL_ARGUMENT;
	Node ptr = map->head;
	while (ptr) {
		Node to_delete = ptr;
		ptr = ptr->next;
		destroyNode(to_delete, map->freeDataElement, map->freeKeyElement);
	}
	return MAP_SUCCESS;
}

void mapDestroy(Map map) {
	if (!map) return;
	mapClear(map);
	free(map);
}


Map mapCopy(Map map) {
	if (map == NULL) return NULL;
	Map map_cpy = mapCreate(map->copyDataElement,
		map->copyKeyElement,
		map->freeDataElement,
		map->freeKeyElement,
		map->compareKeyElements);
	if (mapGetSize(map) == 0) {
		return map_cpy;
	}
	map_cpy->head = copyNode(map->head, map->copyDataElement,
		map->copyKeyElement);
	map_cpy->size = map->size;
	Node old_list_ptr = map->head->next;
	Node new_list_ptr = map_cpy->head;
	while (old_list_ptr) {
		new_list_ptr->next = copyNode(old_list_ptr, map->copyDataElement,
			map->copyKeyElement);
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

//returns a Node element by its key
static Node searchByKey(Map map, MapKeyElement element) {
	if (!map || !element) return NULL;
	Node ptr = map->head;
	while (ptr && map->compareKeyElements(ptr->key_element, element) != 0) {
		ptr = ptr->next;
	}
	return ptr;
}

MapKeyElement mapGetFirst(Map map) {
	if (map == NULL || map->head == NULL) return NULL;
	map->iterator = map->head->key_element;
	return map->head->key_element;
}

MapKeyElement mapGetNext(Map map) {
	if (!map || !map->iterator) return NULL;
	Node ptr = searchByKey(map, map->iterator);
	if (!ptr || !ptr->next) return NULL;
	map->iterator = ptr->next->key_element;
	return ptr->next->key_element;
}

bool mapContains(Map map, MapKeyElement element) {
	if (map == NULL || element == NULL) return NULL;
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
		MapDataElement to_delete = node->data_element;
		node->data_element = dataElement_cpy;
		map->freeDataElement(to_delete);
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
		map->compareKeyElements(current_node->key_element,
			new_node->key_element) < 0) {
		previous_node = current_node;
		current_node = current_node->next;
	}
	if (current_node == map->head) {
		new_node->next = current_node;
		map->head = new_node;
	}
	else if (current_node == NULL || previous_node == current_node) {
		//map reached its final element (or contains only one element)
		previous_node->next = new_node;
	}
	else {
		previous_node->next = new_node;
		new_node->next = current_node;
	}
	map->size++;
	return MAP_SUCCESS;
}

MapDataElement mapGet(Map map, MapKeyElement keyElement) {
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
static MapResult nodeRemove(Map map, Node node) {
	if (!(map && node)) return MAP_NULL_ARGUMENT;
	Node ptr = map->head;
	if (ptr == node) {
		map->head = node->next;
		destroyNode(node, map->freeDataElement, map->freeKeyElement);
		return MAP_SUCCESS;
	}
	while (ptr) {
		if (ptr->next == node) {
			ptr->next = node->next;
			destroyNode(node, map->freeDataElement, map->freeKeyElement);
			return MAP_SUCCESS;
		}
		ptr = ptr->next;
	}
	if (!ptr) return MAP_ITEM_DOES_NOT_EXIST;
	return MAP_SUCCESS;

}


MapResult mapRemove(Map map, MapKeyElement keyElement) {
	if (!(map && keyElement)) return MAP_NULL_ARGUMENT;
	Node node = searchByKey(map, keyElement);
	if (!node) return MAP_ITEM_DOES_NOT_EXIST;
	map->size--;
	return nodeRemove(map, searchByKey(map, keyElement));
}