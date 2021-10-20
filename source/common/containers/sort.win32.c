
#include "common/containers/sort.h"

#include "common/platform/gba/gba.h"


IWRAM_CODE void sort_insertion(sort_index_t* base, const uint32_t num)
{
	sort_index_t* lo = base;
	sort_index_t* hi = base + (num - 1);
	while (hi > lo)
	{
		sort_index_t* max = lo;
		for (sort_index_t* p = lo + 1; p <= hi; p++)
		{
			if (p->value > max->value)
			{
				max = p;
			}
		}
		swap_values(sort_index_t, *max, *hi);
		hi--;
	}
}

