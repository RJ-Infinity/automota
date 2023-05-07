#include "jsExtern.h"

#define TOKENPASTE(x, y) x ## y

#define RJLIST(T) TOKENPASTE(RJList_, T)
typedef struct{
	TYPE* Buffer;
	size_t Length;
	size_t _space;
}RJLIST(TYPE);

#define RJLISTCOPY(T) TOKENPASTE(RJListCopy_, T)
RJLIST(TYPE) RJLISTCOPY(TYPE)(RJLIST(TYPE)* lst){
	RJLIST(TYPE) newLst = {
		.Length = lst->Length,
		._space = lst->_space,
	};
	newLst.Buffer = malloc(newLst._space);
	memcpy(newLst.Buffer,lst->Buffer,newLst._space);
	return newLst;
}


#define RJLISTINCREASESPACE(T) TOKENPASTE(_RJListIncreaseSpace_, T)
void RJLISTINCREASESPACE(TYPE)(RJLIST(TYPE)* lst){
	TYPE* new_Buf = malloc(lst->_space * 2);
	memcpy(new_Buf,lst->Buffer,lst->_space);
	lst->_space *= 2;
	free(lst->Buffer);
	lst->Buffer = new_Buf;
}

#define RJLISTAPPENDAT(T) TOKENPASTE(RJListAppendAt_, T)
bool RJLISTAPPENDAT(TYPE)(RJLIST(TYPE)* lst, TYPE value, size_t index){
	// basic error checks
	if (lst == NULL){return false;}
	if (index == (size_t)-1){index = lst->Length;}
	if (index > lst->Length){return false;}

	if (lst->Buffer == NULL){
		//start out allocating the space for 8 values
		lst->_space = 8 * sizeof *lst->Buffer;
		lst->Buffer = malloc(lst->_space);
	}
	// i.e. if the key array is full
	if (lst->_space == lst->Length * sizeof *lst->Buffer){RJLISTINCREASESPACE(TYPE)(lst);}
	//create a space for the new item
	if (index < lst->Length){memcpy(
		lst->Buffer + index + 1,
		lst->Buffer + index,
		lst->_space - index - 1
	);}
	lst->Buffer[index] = value;
	lst->Length++;
	return true;
}

#define RJLISTAPPEND(T) TOKENPASTE(RJListAppend_, T)
bool RJLISTAPPEND(TYPE)(RJLIST(TYPE)* lst, TYPE value)
{return RJLISTAPPENDAT(TYPE)(lst,value,-1);}

#define RJLISTREMOVE(T) TOKENPASTE(RJListRemove_, T)
bool RJLISTREMOVE(TYPE)(RJLIST(TYPE)* lst,size_t index){
	if (lst == NULL || index >= lst->Length){return false;}
	if (index+1!=lst->Length){memcpy(
		&(lst->Buffer[index]),
		&(lst->Buffer[index+1]),
		(lst->Length-index-1)*(sizeof lst->Buffer[index])
	);}
	lst->Length--;
	return true;
}

#define RJLISTFIND(T) TOKENPASTE(RJListFind_, T)
size_t RJLISTFIND(TYPE)(RJLIST(TYPE)* lst, TYPE item, bool (*cmp)(TYPE, TYPE)){
	for (size_t i = 0; i < lst->Length; i++)
	{if (cmp(lst->Buffer[i],item)){return i;}}
	return (size_t)-1;
}

#define RJLISTDELETE(T) TOKENPASTE(RJListDelete_, T)
void RJLISTDELETE(TYPE)(RJLIST(TYPE)* lst){free(lst->Buffer);}
