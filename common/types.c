// 17 may 2015
#include "../ui.h"
#include "uipriv.h"

struct typeinfo {
	const char *name;
	uintmax_t parent;
	size_t size;
};

static struct ptrArray *types = NULL;

uintmax_t uiRegisterType(const char *name, uintmax_t parent, size_t size)
{
	struct typeinfo *ti;

	if (types == NULL) {
		types = newPtrArray();
		// reserve ID 0
		ptrArrayAppend(types, NULL);
	}
	// TODO prevent our size from being smaller than our parent's
	ti = uiNew(struct typeinfo);
	ti->name = name;
	ti->parent = parent;
	ti->size = size;
	ptrArrayAppend(types, ti);
	return types->len - 1;
}

void *uiIsA(void *p, uintmax_t id, int fail)
{
	uiTyped *t;
	struct typeinfo *ti, *ti2;
	uintmax_t compareTo;

	if (id == 0 || id >= types->len)
		complain("invalid type ID given to uiIsA()");
	t = (uiTyped *) p;
	compareTo = t->Type;
	if (compareTo == 0)
		complain("object %p has no type in uiIsA()", t);
	for (;;) {
		if (compareTo >= types->len)
			complain("invalid type ID in uiIsA()", t);
		if (compareTo == id)
			return t;
		ti = ptrArrayIndex(types, struct typeinfo *, compareTo);
		if (ti->parent == 0)
			break;
		compareTo = ti->parent;
	}
	if (fail) {
		ti = ptrArrayIndex(types, struct typeinfo *, id);
		ti2 = ptrArrayIndex(types, struct typeinfo *, t->Type);
		complain("object %p not a %s in uiIsA() (is a %s)", t, ti->name, ti2->name);
	}
	return NULL;
}

void uninitTypes(void)
{
	struct typeinfo *ti;

	if (types == NULL)		// never initialized; do nothing
		return;
	// the first entry is NULL; get rid of it directly
	ptrArrayDelete(types, 0);
	while (types->len != 0) {
		ti = ptrArrayIndex(types, struct typeinfo *, 0);
		ptrArrayDelete(types, 0);
		uiFree(ti);
	}
	ptrArrayDestroy(types);
}

uiTyped *newTyped(uintmax_t type)
{
	struct typeinfo *ti;
	uiTyped *instance;

	if (type == 0 || type >= types->len)
		complain("invalid type ID given to newTyped()");
	ti = ptrArrayIndex(types, struct typeinfo *, type);
	instance = (uiTyped *) uiAlloc(ti->size, ti->name);
	instance->Type = type;
	return instance;
}
