/*
 * Copyright 2025 Felix P. A. Gillberg HolyBooter
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef holy_PART_HEADER
#define holy_PART_HEADER	1

#include <holy/dl.h>
#include <holy/list.h>

struct holy_disk;

typedef struct holy_partition *holy_partition_t;

#ifdef holy_UTIL
typedef enum
{
  holy_EMBED_PCBIOS
} holy_embed_type_t;
#endif

typedef int (*holy_partition_iterate_hook_t) (struct holy_disk *disk,
					      const holy_partition_t partition,
					      void *data);

/* Partition map type.  */
struct holy_partition_map
{
  /* The next partition map type.  */
  struct holy_partition_map *next;
  struct holy_partition_map **prev;

  /* The name of the partition map type.  */
  const char *name;

  /* Call HOOK with each partition, until HOOK returns non-zero.  */
  holy_err_t (*iterate) (struct holy_disk *disk,
			 holy_partition_iterate_hook_t hook, void *hook_data);
#ifdef holy_UTIL
  /* Determine sectors available for embedding.  */
  holy_err_t (*embed) (struct holy_disk *disk, unsigned int *nsectors,
		       unsigned int max_nsectors,
		       holy_embed_type_t embed_type,
		       holy_disk_addr_t **sectors);
#endif
};
typedef struct holy_partition_map *holy_partition_map_t;

/* Partition description.  */
struct holy_partition
{
  /* The partition number.  */
  int number;

  /* The start sector (relative to parent).  */
  holy_disk_addr_t start;

  /* The length in sector units.  */
  holy_uint64_t len;

  /* The offset of the partition table.  */
  holy_disk_addr_t offset;

  /* The index of this partition in the partition table.  */
  int index;

  /* Parent partition (physically contains this partition).  */
  struct holy_partition *parent;

  /* The type partition map.  */
  holy_partition_map_t partmap;

  /* The type of partition whne it's on MSDOS.
     Used for embedding detection.  */
  holy_uint8_t msdostype;
};

holy_partition_t EXPORT_FUNC(holy_partition_probe) (struct holy_disk *disk,
						    const char *str);
int EXPORT_FUNC(holy_partition_iterate) (struct holy_disk *disk,
					 holy_partition_iterate_hook_t hook,
					 void *hook_data);
char *EXPORT_FUNC(holy_partition_get_name) (const holy_partition_t partition);


extern holy_partition_map_t EXPORT_VAR(holy_partition_map_list);

#ifndef holy_LST_GENERATOR
static inline void
holy_partition_map_register (holy_partition_map_t partmap)
{
  holy_list_push (holy_AS_LIST_P (&holy_partition_map_list),
		  holy_AS_LIST (partmap));
}
#endif

static inline void
holy_partition_map_unregister (holy_partition_map_t partmap)
{
  holy_list_remove (holy_AS_LIST (partmap));
}

#define FOR_PARTITION_MAPS(var) FOR_LIST_ELEMENTS((var), (holy_partition_map_list))


static inline holy_disk_addr_t
holy_partition_get_start (const holy_partition_t p)
{
  holy_partition_t part;
  holy_uint64_t part_start = 0;

  for (part = p; part; part = part->parent)
    part_start += part->start;

  return part_start;
}

static inline holy_uint64_t
holy_partition_get_len (const holy_partition_t p)
{
  return p->len;
}

#endif /* ! holy_PART_HEADER */
