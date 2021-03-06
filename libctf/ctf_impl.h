/*
 * Copyright 2006 -- 2012 Oracle, Inc.
 *
 * Licensed under the GNU General Public License (GPL), version 2. See the file
 * COPYING in the top level of this tree.
 */

#ifndef	_CTF_IMPL_H
#define	_CTF_IMPL_H

#include <sys/errno.h>
#include <sys/ctf_api.h>
#include <sys/types.h>
#include <sys/compiler.h>

#ifdef _KERNEL

#include <sys/systm.h>
#include <sys/cmn_err.h>
#include <sys/varargs.h>

#define	isspace(c) \
	((c) == ' ' || (c) == '\t' || (c) == '\n' || \
	(c) == '\r' || (c) == '\f' || (c) == '\v')

#define	MAP_FAILED	((void *)-1)

#else	/* _KERNEL */

#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>

#endif	/* _KERNEL */

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct ctf_helem {
	uint_t h_name;		/* reference to name in string table */
	ushort_t h_type;	/* corresponding type ID number */
	ushort_t h_next;	/* index of next element in hash chain */
} ctf_helem_t;

typedef struct ctf_hash {
	ushort_t *h_buckets;	/* hash bucket array (chain indices) */
	ctf_helem_t *h_chains;	/* hash chains buffer */
	ushort_t h_nbuckets;	/* number of elements in bucket array */
	ushort_t h_nelems;	/* number of elements in hash table */
	uint_t h_free;		/* index of next free hash element */
} ctf_hash_t;

typedef struct ctf_strs {
	const char *cts_strs;	/* base address of string table */
	size_t cts_len;		/* size of string table in bytes */
} ctf_strs_t;

typedef struct ctf_dmodel {
	const char *ctd_name;	/* data model name */
	int ctd_code;		/* data model code */
	size_t ctd_pointer;	/* size of void * in bytes */
	size_t ctd_char;	/* size of char in bytes */
	size_t ctd_short;	/* size of short in bytes */
	size_t ctd_int;		/* size of int in bytes */
	size_t ctd_long;	/* size of long in bytes */
} ctf_dmodel_t;

typedef struct ctf_lookup {
	const char *ctl_prefix;	/* string prefix for this lookup */
	size_t ctl_len;		/* length of prefix string in bytes */
	ctf_hash_t *ctl_hash;	/* pointer to hash table for lookup */
} ctf_lookup_t;

typedef struct ctf_fileops {
	ushort_t (*ctfo_get_kind)(ushort_t);
	ushort_t (*ctfo_get_root)(ushort_t);
	ushort_t (*ctfo_get_vlen)(ushort_t);
} ctf_fileops_t;

typedef struct ctf_list {
	struct ctf_list *l_prev; /* previous pointer or tail pointer */
	struct ctf_list *l_next; /* next pointer or head pointer */
} ctf_list_t;

typedef enum {
	CTF_PREC_BASE,
	CTF_PREC_POINTER,
	CTF_PREC_ARRAY,
	CTF_PREC_FUNCTION,
	CTF_PREC_MAX
} ctf_decl_prec_t;

typedef struct ctf_decl_node {
	ctf_list_t cd_list;			/* linked list pointers */
	ctf_id_t cd_type;			/* type identifier */
	uint_t cd_kind;				/* type kind */
	uint_t cd_n;				/* type dimension if array */
} ctf_decl_node_t;

typedef struct ctf_decl {
	ctf_list_t cd_nodes[CTF_PREC_MAX];	/* declaration node stacks */
	int cd_order[CTF_PREC_MAX];		/* storage order of decls */
	ctf_decl_prec_t cd_qualp;		/* qualifier precision */
	ctf_decl_prec_t cd_ordp;		/* ordered precision */
	char *cd_buf;				/* buffer for output */
	char *cd_ptr;				/* buffer location */
	char *cd_end;				/* buffer limit */
	size_t cd_len;				/* buffer space required */
	int cd_err;				/* saved error value */
} ctf_decl_t;

typedef struct ctf_dmdef {
	ctf_list_t dmd_list;	/* list forward/back pointers */
	char *dmd_name;		/* name of this member */
	ctf_id_t dmd_type;	/* type of this member (for sou) */
	ulong_t dmd_offset;	/* offset of this member in bits (for sou) */
	int dmd_value;		/* value of this member (for enum) */
} ctf_dmdef_t;

typedef struct ctf_dtdef {
	ctf_list_t dtd_list;	/* list forward/back pointers */
	struct ctf_dtdef *dtd_hash; /* hash chain pointer for ctf_dthash */
	char *dtd_name;		/* name associated with definition (if any) */
	ctf_id_t dtd_type;	/* type identifier for this definition */
	ctf_type_t dtd_data;	/* type node (see <sys/ctf.h>) */
	union {
		ctf_list_t dtu_members;	/* struct, union, or enum */
		ctf_arinfo_t dtu_arr;	/* array */
		ctf_encoding_t dtu_enc;	/* integer or float */
		ctf_id_t *dtu_argv;	/* function */
	} dtd_u;
} ctf_dtdef_t;

typedef struct ctf_dvdef {
	ctf_list_t dvd_list;	/* list forward/back pointers */
	struct ctf_dvdef *dvd_hash; /* hash chain pointer for ctf_dvhash */
	char *dvd_name;		/* name associated with variable */
	ctf_id_t dvd_type;	/* type of variable */
	ulong_t dvd_snapshots;	/* snapshot count when inserted */
} ctf_dvdef_t;

typedef struct ctf_bundle {
	ctf_file_t *ctb_file;	/* CTF container handle */
	ctf_id_t ctb_type;	/* CTF type identifier */
	ctf_dtdef_t *ctb_dtd;	/* CTF dynamic type definition (if any) */
} ctf_bundle_t;

/*
 * The ctf_file is the structure used to represent a CTF container to library
 * clients, who see it only as an opaque pointer.  Modifications can therefore
 * be made freely to this structure without regard to client versioning.  The
 * ctf_file_t typedef appears in <sys/ctf_api.h> and declares a forward tag.
 *
 * NOTE: ctf_update() requires that everything inside of ctf_file either be an
 * immediate value, a pointer to dynamically allocated data *outside* of the
 * ctf_file itself, or a pointer to statically allocated data.  If you add a
 * pointer to ctf_file that points to something within the ctf_file itself,
 * you must make corresponding changes to ctf_update().
 */
struct ctf_file {
	const ctf_fileops_t *ctf_fileops; /* version-specific file operations */
	ctf_sect_t ctf_data;	/* CTF data from object file */
	ctf_sect_t ctf_symtab;	/* symbol table from object file */
	ctf_sect_t ctf_strtab;	/* string table from object file */
	ctf_hash_t ctf_structs;	/* hash table of struct types */
	ctf_hash_t ctf_unions;	/* hash table of union types */
	ctf_hash_t ctf_enums;	/* hash table of enum types */
	ctf_hash_t ctf_names;	/* hash table of remaining type names */
	ctf_lookup_t ctf_lookups[5];	/* pointers to hashes for name lookup */
	ctf_strs_t ctf_str[2];	/* array of string table base and bounds */
	const uchar_t *ctf_base; /* base of CTF header + uncompressed buffer */
	const uchar_t *ctf_buf;	/* uncompressed CTF data buffer */
	size_t ctf_size;	/* size of CTF header + uncompressed data */
	uint_t *ctf_sxlate;	/* translation table for symtab entries */
	ulong_t ctf_nsyms;	/* number of entries in symtab xlate table */
	uint_t *ctf_txlate;	/* translation table for type IDs */
	ushort_t *ctf_ptrtab;	/* translation table for pointer-to lookups */
	struct ctf_varent *ctf_vars;	/* sorted variable->type mapping */
	ulong_t ctf_nvars;		/* number of variables in ctf_vars */
	ulong_t ctf_typemax;	/* maximum valid type ID number */
	const ctf_dmodel_t *ctf_dmodel;	/* data model pointer (see above) */
	struct ctf_file *ctf_parent;	/* parent CTF container (if any) */
	const char *ctf_parlabel;	/* label in parent container (if any) */
	const char *ctf_parname;	/* basename of parent (if any) */
	char *ctf_dynparname;	/* dynamically allocated name of parent */
	uint_t ctf_refcnt;	/* reference count (for parent links) */
	uint_t ctf_flags;	/* libctf flags (see below) */
	int ctf_errno;		/* error code for most recent error */
	int ctf_version;	/* CTF data version */
	ctf_dtdef_t **ctf_dthash; /* hash of dynamic type definitions */
	ulong_t ctf_dthashlen;	/* size of dynamic type hash bucket array */
	ctf_list_t ctf_dtdefs;	/* list of dynamic type definitions */
	ctf_dvdef_t **ctf_dvhash; /* hash of dynamic variable mappings */
	ulong_t ctf_dvhashlen;	  /* size of dynvar hash bucket array */
	ctf_list_t ctf_dvdefs;	  /* list of dynamic variable definitions */
	size_t ctf_dtvstrlen;	/* total length of dynamic type+var strings */
	ulong_t ctf_dtnextid;	/* next dynamic type id to assign */
	ulong_t ctf_dtoldid;	/* oldest id that has been committed */
	ulong_t ctf_snapshots;	/* ctf_snapshot() plus ctf_update() count */
	ulong_t ctf_snapshot_lu; /* ctf_snapshot() call count at last update */
	void *ctf_specific;	/* data for ctf_get/setspecific */
};

#define	LCTF_INDEX_TO_TYPEPTR(fp, i) \
	((ctf_type_t *)((uintptr_t)(fp)->ctf_buf + (fp)->ctf_txlate[(i)]))

#define	LCTF_INFO_KIND(fp, info)	((fp)->ctf_fileops->ctfo_get_kind(info))
#define	LCTF_INFO_ROOT(fp, info)	((fp)->ctf_fileops->ctfo_get_root(info))
#define	LCTF_INFO_VLEN(fp, info)	((fp)->ctf_fileops->ctfo_get_vlen(info))

#define	LCTF_MMAP	0x0001	/* libctf should munmap buffers on close */
#define	LCTF_CHILD	0x0002	/* CTF container is a child */
#define	LCTF_RDWR	0x0004	/* CTF container is writable */
#define	LCTF_DIRTY	0x0008	/* CTF container has been modified */

extern ssize_t ctf_get_ctt_size(const ctf_file_t *, const ctf_type_t *,
    ssize_t *, ssize_t *);

extern const ctf_type_t *ctf_lookup_by_id(ctf_file_t **, ctf_id_t);

extern int ctf_hash_create(ctf_hash_t *, ulong_t);
extern int ctf_hash_insert(ctf_hash_t *, ctf_file_t *, ushort_t, uint_t);
extern int ctf_hash_define(ctf_hash_t *, ctf_file_t *, ushort_t, uint_t);
extern ctf_helem_t *ctf_hash_lookup(ctf_hash_t *, ctf_file_t *,
    const char *, size_t);
extern uint_t ctf_hash_size(const ctf_hash_t *);
extern ulong_t ctf_hash_compute(const char *key, size_t len);
extern void ctf_hash_destroy(ctf_hash_t *);

#define	ctf_list_prev(elem)	((void *)(((ctf_list_t *)(elem))->l_prev))
#define	ctf_list_next(elem)	((void *)(((ctf_list_t *)(elem))->l_next))

extern void ctf_list_append(ctf_list_t *, void *);
extern void ctf_list_prepend(ctf_list_t *, void *);
extern void ctf_list_delete(ctf_list_t *, void *);

extern void ctf_dtd_insert(ctf_file_t *, ctf_dtdef_t *);
extern void ctf_dtd_delete(ctf_file_t *, ctf_dtdef_t *);
extern ctf_dtdef_t *ctf_dtd_lookup(ctf_file_t *, ctf_id_t);

extern void ctf_dvd_insert(ctf_file_t *, ctf_dvdef_t *);
extern void ctf_dvd_delete(ctf_file_t *, ctf_dvdef_t *);
extern ctf_dvdef_t *ctf_dvd_lookup(ctf_file_t *, const char *);

extern void ctf_decl_init(ctf_decl_t *, char *, size_t);
extern void ctf_decl_fini(ctf_decl_t *);
extern void ctf_decl_push(ctf_decl_t *, ctf_file_t *, ctf_id_t);

_dt_printflike_(2,3)
extern void ctf_decl_sprintf(ctf_decl_t *, const char *, ...);

extern const char *ctf_strraw(ctf_file_t *, uint_t);
extern const char *ctf_strptr(ctf_file_t *, uint_t);

extern ctf_file_t *ctf_set_open_errno(int *, int);
extern long ctf_set_errno(ctf_file_t *, int);

extern const void *ctf_sect_mmap(ctf_sect_t *, int);
extern void ctf_sect_munmap(const ctf_sect_t *);

extern void *ctf_data_alloc(size_t);
extern void ctf_data_free(void *, size_t);
extern void ctf_data_protect(void *, size_t);

extern void *ctf_alloc(size_t);
extern void ctf_free(void *, size_t);

extern char *ctf_strdup(const char *);
extern const char *ctf_strerror(int);

_dt_printflike_(1,2)
extern void ctf_dprintf(const char *, ...);

extern const char _CTF_SECTION[];	/* name of CTF ELF section */
extern const char _CTF_NULLSTR[];	/* empty string */

extern int _libctf_version;		/* library client version */
extern int _libctf_debug;		/* debugging messages enabled */

#ifdef	__cplusplus
}
#endif

#endif	/* _CTF_IMPL_H */
