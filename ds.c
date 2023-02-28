#include "_WQ.h"

/*------------------------------------------------------------------
 * list.
 */
WQ_API void _list_link(void **head, void *ptr)
{
	_list_element *current = NULL;
	__check_address(ptr);
	__check_address(head);

	if(*head == NULL)
	{
		*head = ptr;
		__to_list_element(*head)->next = NULL;
		return;
	}
	current = __to_list_element(*head);
	while(current->next)
	{
		current = current->next;
	}
	current->next = __to_list_element(ptr);
}
WQ_API void *_list_find(void **head, _bool_function func, void *par)
{
	__check_address_and_return(head, NULL);
	__check_address_and_return(func, NULL);
	_list_element *current = __to_list_element(*head);
	while(current)
	{
		if(func(__to_void(current), par))
		{
			return current;
		}
		current = current->next;
	}
	return NULL;
}
WQ_API void *_list_unlink(void **head, _bool_function func, void *par)
{
	__check_address_and_return(head, NULL);
	__check_address_and_return(func, NULL);
	_list_element *current = __to_list_element(*head);
	_list_element *previous = NULL;
	while(current)
	{
		if(func(__to_void(current), par))
		{
			if(!previous)
			{
				*head = current->next;
				current->next = NULL;
				return __to_void(current);
			}
			previous->next = current->next;
			current->next = NULL;
			return __to_void(current);
		}
		previous = current;
		current = current->next;
	}
	return NULL;
}
WQ_API unsigned int _list_length(void **head)
{
	__check_address_and_return(head, 0);
	_list_element *current = __to_list_element(*head);
	unsigned int  length = 0;

	while(current)
	{
		current = current->next;
		length++;
	}
	return length;
}
WQ_API void _list_clone(void **head, void **clone_head, _clone_function func)
{
	__check_address(head);
	__check_address(func);

	_list_element *current = __to_list_element(*head);
	while(current)
	{
		void *ptr = func(__to_void(current));
		if (ptr)
		{
			__to_list_element(ptr)->next = NULL;
			_list_link(clone_head, ptr);
		}
		current = current->next;
	}
}
WQ_API void _list_clear(void **head, _clear_function func)
{
	__check_address(head);
	__check_address(func);

	_list_element *current = __to_list_element(*head);
	_list_element *next = *head ? __to_list_element(*head)->next : NULL;
	while(current)
	{
		func(current);
		current = next;
		next = current ? current->next : NULL;
	}
	*head = NULL;
}
WQ_API void _list_look(void **head, _void_function func)
{
	__check_address(head);
	__check_address(func);

	_list_element *current = __to_list_element(*head);
	while(current)
	{
		func(current);
		current = current->next;
	}
}
WQ_API BOOL _list_empty(void **head)
{
	return (_list_length(head) <= 0);
}

/*------------------------------------------------------------------
 * doublelist
 */
WQ_API void _doublelist_link(void **head, void *ptr)
{
	__check_address(head);
	__check_address(ptr);

	if(*head == NULL)
	{
		*head = ptr;
		return;
	}
	_doublelist_element *current = __to_doublelist_element(*head);
	while(current->next)
	{
		current = current->next;
	}

	current->next = __to_doublelist_element(ptr);
	__to_doublelist_element(ptr)->previous = current;
}
WQ_API void *_doublelist_unlink(void **head, _bool_function func, void *par)
{
	__check_address_and_return(head, NULL);
	__check_address_and_return(func, NULL);

	_doublelist_element *current = __to_doublelist_element(*head);
	while(current)
	{
		if(func(current, par))
		{
			if(!current->previous)
			{
				*head = current->next;
				__to_doublelist_element(*head)->previous = NULL;
				current->previous = NULL;
				current->next = NULL;
				return __to_void(current);
			}
			current->previous->next = current->next;
			if(current->next)
			{
				current->next->previous = current->previous;
			}
			current->previous = NULL;
			current->next = NULL;
			return __to_void(current);
		}
		current = current->next;
	}
	return NULL;
}
WQ_API void *_doublelist_find(void *ptr, _bool_function func, void *par, doublelist_direction dir)
{
	__check_address_and_return(ptr, NULL);
	__check_address_and_return(func, NULL);

	_doublelist_element *current = __to_doublelist_element(ptr);

	while(current)
	{
		if(func(current, par))
		{
			return __to_void(current);
		}
		((dir == doublelist_direction_next) ? (current = current->next) :
				(current = current->previous));
	}
	return NULL;
}
WQ_API void _doublelist_clear(void **head, _clear_function func)
{
	__check_address(head);
	__check_address(func);

	_doublelist_element *current = __to_doublelist_element(*head);
	_doublelist_element *next = *head ? __to_doublelist_element(*head)->next : NULL;
	while(current)
	{
		func(current);
		current = next;
		next = current ? current->next : NULL;
	}
	*head = NULL;
}
WQ_API unsigned int _doublelist_length(void **head)
{
	__check_address_and_return(head, 0);
	_doublelist_element *current = __to_doublelist_element(*head);
	unsigned int  length = 0;

	while(current)
	{
		current = current->next;
		length++;
	}
	return length;
}
WQ_API BOOL _doublelist_empty(void **head)
{
	return (_doublelist_length(head) <= 0);
}
WQ_API void _doublelist_look(void **head, _void_function func)
{
	__check_address(head);
	__check_address(func);

	_doublelist_element *current = __to_doublelist_element(*head);
	while(current)
	{
		func(current);
		current = current->next;
	}
}
WQ_API void *doublelist_previous(void *ptr)
{
	__check_address_and_return(ptr, NULL);
	return __to_void(__to_doublelist_element(ptr)->previous);
}
WQ_API void *doublelist_next(void *ptr)
{
	__check_address_and_return(ptr, NULL);
	return __to_void(__to_doublelist_element(ptr)->next);
}
/*------------------------------------------------------------------
 * stack
 */
WQ_API void _stack_push(void **head, void *ptr)
{
	__check_address(head);
	__check_address(ptr);

	if(*head == NULL)
	{
		*head = ptr;
		return;
	}
	_stack_element *top = __to_stack_element(*head);
	*head = ptr;
	__to_stack_element(ptr)->previous = top;

}
WQ_API void *_stack_pop(void **head)
{
	__check_address_and_return(head, NULL);
	void *ptr = *head;
	if(ptr)
	{
		*head = __to_void(__to_stack_element(ptr)->previous);
	}

	return ptr;
}
WQ_API unsigned int _stack_length(void **head)
{
	__check_address_and_return(head, 0);
	_stack_element *current = __to_stack_element(*head);
	unsigned int  length = 0;

	while(current)
	{
		current = current->previous;
		length++;
	}
	return length;
}
WQ_API BOOL _stack_empty(void **head)
{
	return _stack_length(head) <= 0;
}
WQ_API void _stack_clear(void **head, _clear_function func)
{
	__check_address(head);
	_stack_element *current = NULL;
	while((current = __to_stack_element(_stack_pop(head))) != NULL)
	{
		func(current);
	}
}
WQ_API void _stack_look(void **head, _void_function func)
{
	__check_address(head);
	_stack_element *current = __to_stack_element(*head);
	while(current)
	{
		func(current);
		current = current->previous;
	}
}
/*------------------------------------------------------------------
 * queue
 */
WQ_API void _queue_push(void **head, void *ptr)
{
	_queue_element *current = NULL;
	__check_address(ptr);
	__check_address(head);

	if(*head == NULL)
	{
		*head = ptr;
		return;
	}
	current = __to_queue_element(*head);
	while(current->next)
	{
		current = current->next;
	}
	current->next = __to_queue_element(ptr);
}
WQ_API void *_queue_pop(void **head)
{
	__check_address_and_return(head, NULL);
	void *ptr = *head;
	if(ptr)
	{
		*head = __to_void(__to_queue_element(ptr)->next);
	}
	return ptr;
}
WQ_API void _queue_clear(void **head, _clear_function func)
{
	__check_address(head);
	_queue_element *current = NULL;
	while((current = __to_queue_element(_queue_pop(head))) != NULL)
	{
		func(current);
	}
}
WQ_API unsigned int _queue_length(void **head)
{
	__check_address_and_return(head, 0);
	_queue_element *current = __to_queue_element(*head);
	unsigned int  length = 0;

	while(current)
	{
		current = current->next;
		length++;
	}
	return length;
}
WQ_API BOOL _queue_empty(void **head)
{
	return _queue_length(head) <= 0;
}
WQ_API void _queue_look(void **head, _void_function func)
{
	__check_address(head);
	_queue_element *current = __to_queue_element(*head);
	while(current)
	{
		func(current);
		current = current->next;
	}
}
/*------------------------------------------------------------------
 * queue
 */
WQ_API void _tree_link(void **root, void *parent, void *ptr)
{
	__check_address(ptr);
	__check_address(root);

	if(*root == NULL)
	{
		*root = ptr;
		return;
	}
	if(parent)
	{
		__to_tree_element(ptr)->parent = __to_tree_element(parent);
		if(__to_tree_element(parent)->child == NULL)
		{
			__to_tree_element(parent)->child = __to_tree_element(ptr);
			return;
		}
		_tree_element *current = __to_tree_element(parent)->child;
		while(current->sibling)
		{
			current = current->sibling;
		}
		current->sibling = __to_tree_element(ptr);
	}
}
WQ_API void *_tree_find(void *parent, _bool_function func, void *par)
{
	void *ptr = parent;
	void *_ptr = NULL;
	if(ptr)
	{
		_tree_element *current = __to_tree_element(ptr);
		if(func(ptr, par)) return ptr;

		if(current->child) 				_ptr = _tree_find(current->child, func, par);
		if(!_ptr && current->sibling) 	_ptr = _tree_find(current->sibling, func, par);
	}
	return _ptr;
}
WQ_API void *_tree_unlink(void **root, void *parent, _bool_function func, void *par)
{
	__check_address_and_return(root, NULL);
	__check_address_and_return(func, NULL);
	_tree_element *current = NULL;
	_tree_element *previous = NULL;

	void *ptr = _tree_find(parent ? parent : *root, func, par);
	if(ptr)
	{
		if(ptr == *root)
		{
			*root = NULL;
		}
		else
		{
			current = __to_tree_element(ptr)->parent->child;
			while(current)
			{
				if(current == ptr)
				{
					if(!previous) 	current->parent->child = current->sibling;
					else 			previous->sibling = current->sibling;
				}
				previous = current;
				current = current->sibling;
			}
		}
		__to_tree_element(ptr)->parent = NULL;
		__to_tree_element(ptr)->sibling = NULL;
	}
	return ptr;
}


WQ_API void _tree_clear(void **parent, _clear_function func)
{
	__check_address(parent);
	void *ptr = *parent;
	if(ptr)
	{
		_tree_element *current = __to_tree_element(ptr);
		if(current->child) 		_tree_clear((void **)&(current->child), func);
		if(current->sibling) 	_tree_clear((void **)&(current->sibling), func);
		func(ptr);
		*parent = NULL;
	}
}
WQ_API void *_tree_parent(void *ptr)
{
	__check_address_and_return(ptr, NULL);
	return __to_void(__to_tree_element(ptr)->parent);
}
WQ_API void *_tree_next_child(void *parent, void *ptr)
{
	__check_address_and_return(parent, NULL);
	if(!ptr) return __to_void(__to_tree_element(parent)->child);
	return __to_void(__to_tree_element(ptr)->sibling);
}
WQ_API BOOL _tree_have_child(void *parent)
{
	return _tree_next_child(parent, NULL) != NULL;
}
WQ_API void _tree_look(void *parent, _void_function func)
{
	__check_address(parent);
	void *ptr = parent;

	_tree_element *current = __to_tree_element(ptr);
	if(current->child) 		_tree_look((current->child), func);
	if(current->sibling) 	_tree_look((current->sibling), func);
	func(ptr);

}

