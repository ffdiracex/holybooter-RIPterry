/*
 * Copyright 2025 Felix P. A. Gillberg HolyBooter
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef holy_DISK_HEADER
#define holy_DISK_HEADER	1

#include <config.h>

#include <holy/symbol.h>
#include <holy/err.h>
#include <holy/types.h>
#include <holy/device.h>
/* For NULL.  */
#include <holy/mm.h>

/* These are used to set a device id. When you add a new disk device,
   you must define a new id for it here.  */
enum holy_disk_dev_id
  {
    holy_DISK_DEVICE_BIOSDISK_ID,
    holy_DISK_DEVICE_OFDISK_ID,
    holy_DISK_DEVICE_LOOPBACK_ID,
    holy_DISK_DEVICE_EFIDISK_ID,
    holy_DISK_DEVICE_DISKFILTER_ID,
    holy_DISK_DEVICE_HOST_ID,
    holy_DISK_DEVICE_ATA_ID,
    holy_DISK_DEVICE_MEMDISK_ID,
    holy_DISK_DEVICE_NAND_ID,
    holy_DISK_DEVICE_SCSI_ID,
    holy_DISK_DEVICE_CRYPTODISK_ID,
    holy_DISK_DEVICE_ARCDISK_ID,
    holy_DISK_DEVICE_HOSTDISK_ID,
    holy_DISK_DEVICE_PROCFS_ID,
    holy_DISK_DEVICE_CBFSDISK_ID,
    holy_DISK_DEVICE_UBOOTDISK_ID,
    holy_DISK_DEVICE_XEN,
  };

struct holy_disk;
#ifdef holy_UTIL
struct holy_disk_memberlist;
#endif

typedef enum
  { 
    holy_DISK_PULL_NONE,
    holy_DISK_PULL_REMOVABLE,
    holy_DISK_PULL_RESCAN,
    holy_DISK_PULL_MAX
  } holy_disk_pull_t;

typedef int (*holy_disk_dev_iterate_hook_t) (const char *name, void *data);

/* Disk device.  */
struct holy_disk_dev
{
  /* The device name.  */
  const char *name;

  /* The device id used by the cache manager.  */
  enum holy_disk_dev_id id;

  /* Call HOOK with each device name, until HOOK returns non-zero.  */
  int (*iterate) (holy_disk_dev_iterate_hook_t hook, void *hook_data,
		  holy_disk_pull_t pull);

  /* Open the device named NAME, and set up DISK.  */
  holy_err_t (*open) (const char *name, struct holy_disk *disk);

  /* Close the disk DISK.  */
  void (*close) (struct holy_disk *disk);

  /* Read SIZE sectors from the sector SECTOR of the disk DISK into BUF.  */
  holy_err_t (*read) (struct holy_disk *disk, holy_disk_addr_t sector,
		      holy_size_t size, char *buf);

  /* Write SIZE sectors from BUF into the sector SECTOR of the disk DISK.  */
  holy_err_t (*write) (struct holy_disk *disk, holy_disk_addr_t sector,
		       holy_size_t size, const char *buf);

#ifdef holy_UTIL
  struct holy_disk_memberlist *(*memberlist) (struct holy_disk *disk);
  const char * (*raidname) (struct holy_disk *disk);
#endif

  /* The next disk device.  */
  struct holy_disk_dev *next;
};
typedef struct holy_disk_dev *holy_disk_dev_t;

extern holy_disk_dev_t EXPORT_VAR (holy_disk_dev_list);

struct holy_partition;

typedef void (*holy_disk_read_hook_t) (holy_disk_addr_t sector,
				       unsigned offset, unsigned length,
				       void *data);

/* Disk.  */
struct holy_disk
{
  /* The disk name.  */
  const char *name;

  /* The underlying disk device.  */
  holy_disk_dev_t dev;

  /* The total number of sectors.  */
  holy_uint64_t total_sectors;

  /* Logarithm of sector size.  */
  unsigned int log_sector_size;

  /* Maximum number of sectors read divided by holy_DISK_CACHE_SIZE.  */
  unsigned int max_agglomerate;

  /* The id used by the disk cache manager.  */
  unsigned long id;

  /* The partition information. This is machine-specific.  */
  struct holy_partition *partition;

  /* Called when a sector was read. OFFSET is between 0 and
     the sector size minus 1, and LENGTH is between 0 and the sector size.  */
  holy_disk_read_hook_t read_hook;

  /* Caller-specific data passed to the read hook.  */
  void *read_hook_data;

  /* Device-specific data.  */
  void *data;
};
typedef struct holy_disk *holy_disk_t;

#ifdef holy_UTIL
struct holy_disk_memberlist
{
  holy_disk_t disk;
  struct holy_disk_memberlist *next;
};
typedef struct holy_disk_memberlist *holy_disk_memberlist_t;
#endif

/* The sector size.  */
#define holy_DISK_SECTOR_SIZE	0x200
#define holy_DISK_SECTOR_BITS	9

/* The maximum number of disk caches.  */
#define holy_DISK_CACHE_NUM	1021

/* The size of a disk cache in 512B units. Must be at least as big as the
   largest supported sector size, currently 16K.  */
#define holy_DISK_CACHE_BITS	6
#define holy_DISK_CACHE_SIZE	(1 << holy_DISK_CACHE_BITS)

#define holy_DISK_MAX_MAX_AGGLOMERATE ((1 << (30 - holy_DISK_CACHE_BITS - holy_DISK_SECTOR_BITS)) - 1)

/* Return value of holy_disk_get_size() in case disk size is unknown. */
#define holy_DISK_SIZE_UNKNOWN	 0xffffffffffffffffULL

/* This is called from the memory manager.  */
void holy_disk_cache_invalidate_all (void);

void EXPORT_FUNC(holy_disk_dev_register) (holy_disk_dev_t dev);
void EXPORT_FUNC(holy_disk_dev_unregister) (holy_disk_dev_t dev);
static inline int
holy_disk_dev_iterate (holy_disk_dev_iterate_hook_t hook, void *hook_data)
{
  holy_disk_dev_t p;
  holy_disk_pull_t pull;

  for (pull = 0; pull < holy_DISK_PULL_MAX; pull++)
    for (p = holy_disk_dev_list; p; p = p->next)
      if (p->iterate && (p->iterate) (hook, hook_data, pull))
	return 1;

  return 0;
}

holy_disk_t EXPORT_FUNC(holy_disk_open) (const char *name);
void EXPORT_FUNC(holy_disk_close) (holy_disk_t disk);
holy_err_t EXPORT_FUNC(holy_disk_read) (holy_disk_t disk,
					holy_disk_addr_t sector,
					holy_off_t offset,
					holy_size_t size,
					void *buf);
holy_err_t holy_disk_write (holy_disk_t disk,
			    holy_disk_addr_t sector,
			    holy_off_t offset,
			    holy_size_t size,
			    const void *buf);
extern holy_err_t (*EXPORT_VAR(holy_disk_write_weak)) (holy_disk_t disk,
						       holy_disk_addr_t sector,
						       holy_off_t offset,
						       holy_size_t size,
						       const void *buf);


holy_uint64_t EXPORT_FUNC(holy_disk_get_size) (holy_disk_t disk);

#if DISK_CACHE_STATS
void
EXPORT_FUNC(holy_disk_cache_get_performance) (unsigned long *hits, unsigned long *misses);
#endif

extern void (* EXPORT_VAR(holy_disk_firmware_fini)) (void);
extern int EXPORT_VAR(holy_disk_firmware_is_tainted);

static inline void
holy_stop_disk_firmware (void)
{
  /* To prevent two drivers operating on the same disks.  */
  holy_disk_firmware_is_tainted = 1;
  if (holy_disk_firmware_fini)
    {
      holy_disk_firmware_fini ();
      holy_disk_firmware_fini = NULL;
    }
}

/* Disk cache.  */
struct holy_disk_cache
{
  enum holy_disk_dev_id dev_id;
  unsigned long disk_id;
  holy_disk_addr_t sector;
  char *data;
  int lock;
};

extern struct holy_disk_cache EXPORT_VAR(holy_disk_cache_table)[holy_DISK_CACHE_NUM];

#if defined (holy_UTIL)
void holy_lvm_init (void);
void holy_ldm_init (void);
void holy_mdraid09_init (void);
void holy_mdraid1x_init (void);
void holy_diskfilter_init (void);
void holy_lvm_fini (void);
void holy_ldm_fini (void);
void holy_mdraid09_fini (void);
void holy_mdraid1x_fini (void);
void holy_diskfilter_fini (void);
#endif

#endif /* ! holy_DISK_HEADER */
