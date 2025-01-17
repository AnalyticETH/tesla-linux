/*
* Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2010 - 2020, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
*/

#include "ia_css_psys_process.h"
#include "ia_css_psys_dynamic_storage_class.h"
#include "ia_css_psys_process_private_types.h"
#include <misc_support.h>	/* for NOT_USED */

/*
 * Functions to possibly inline
 */

#ifndef __IA_CSS_PSYS_DYNAMIC_INLINE__
#include "ia_css_psys_process_impl.h"
#endif /* __IA_CSS_PSYS_DYNAMIC_INLINE__ */

/*
 * Functions not to inline
 */

/* This source file is created with the intention of sharing and
 * compiled for host and firmware. Since there is no native 64bit
 * data type support for firmware this wouldn't compile for SP
 * tile. The part of the file that is not compilable are marked
 * with the following __HIVECC marker and this comment. Once we
 * come up with a solution to address this issue this will be
 * removed.
 */
#if !defined(__HIVECC)
size_t ia_css_sizeof_process(
	const ia_css_program_manifest_t			*manifest,
	const ia_css_program_param_t			*param)
{
	size_t	size = 0, tmp_size;

	uint8_t	program_dependency_count;
	uint8_t terminal_dependency_count;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_sizeof_process(): enter:\n");

	COMPILATION_ERROR_IF(
		SIZE_OF_PROCESS_STRUCT_BITS !=
			(CHAR_BIT * sizeof(ia_css_process_t)));

	COMPILATION_ERROR_IF(0 != sizeof(ia_css_process_t)%sizeof(uint64_t));

	verifexit(manifest != NULL);
	verifexit(param != NULL);

	size += sizeof(ia_css_process_t);

	program_dependency_count =
		ia_css_program_manifest_get_program_dependency_count(manifest);
	terminal_dependency_count =
		ia_css_program_manifest_get_terminal_dependency_count(manifest);

	tmp_size = program_dependency_count*sizeof(vied_nci_resource_id_t);
	size += tot_bytes_for_pow2_align(sizeof(uint64_t), tmp_size);
	tmp_size = terminal_dependency_count*sizeof(uint8_t);
	size += tot_bytes_for_pow2_align(sizeof(uint64_t), tmp_size);

EXIT:
	if (NULL == manifest || NULL == param) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_sizeof_process invalid argument\n");
	}
	return size;
}

ia_css_process_t *ia_css_process_create(
	void					*raw_mem,
	const ia_css_program_manifest_t		*manifest,
	const ia_css_program_param_t		*param,
	const uint32_t				program_idx)
{
	size_t	tmp_size;
	int retval = -1;
	ia_css_process_t	*process = NULL;
	char *process_raw_ptr = (char *) raw_mem;

	/* size_t	size = ia_css_sizeof_process(manifest, param); */
	uint8_t	program_dependency_count;
	uint8_t	terminal_dependency_count;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO,
		"ia_css_process_create(): enter:\n");

	verifexit(manifest != NULL);
	verifexit(param != NULL);
	verifexit(process_raw_ptr != NULL);

	process = (ia_css_process_t *) process_raw_ptr;
	verifexit(process != NULL);

	process->kernel_bitmap =
		ia_css_program_manifest_get_kernel_bitmap(manifest);
	process->state = IA_CSS_PROCESS_CREATED;

	program_dependency_count =
		ia_css_program_manifest_get_program_dependency_count(manifest);
	terminal_dependency_count =
		ia_css_program_manifest_get_terminal_dependency_count(manifest);

	/* A process requires at least one input or output */
	verifexit((program_dependency_count +
		   terminal_dependency_count) != 0);

	process_raw_ptr += sizeof(ia_css_process_t);
	if (program_dependency_count != 0) {
		process->cell_dependencies_offset =
			(uint16_t) (process_raw_ptr - (char *)process);
		tmp_size =
		      program_dependency_count * sizeof(vied_nci_resource_id_t);
		process_raw_ptr +=
			tot_bytes_for_pow2_align(sizeof(uint64_t), tmp_size);
	} else {
		process->cell_dependencies_offset = 0;
	}

	if (terminal_dependency_count != 0) {
		process->terminal_dependencies_offset =
			(uint16_t) (process_raw_ptr - (char *)process);
	}

	process->size = (uint32_t)ia_css_sizeof_process(manifest, param);

	process->ID = ia_css_program_manifest_get_program_ID(manifest);
	verifexit(process->ID != 0);
	process->program_idx = program_idx;

	process->cell_dependency_count = program_dependency_count;
	process->terminal_dependency_count = terminal_dependency_count;

	process->parent_offset = 0;

	verifexit(ia_css_process_clear_all(process) == 0);

	process->state = IA_CSS_PROCESS_READY;
	retval = 0;

	IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO,
		"ia_css_process_create(): Created successfully process %p ID 0x%x\n",
		process, process->ID);

EXIT:
	if (NULL == manifest || NULL == param) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_create invalid argument\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_create failed (%i)\n", retval);
		process = ia_css_process_destroy(process);
	}
	return process;
}

ia_css_process_t *ia_css_process_destroy(
	ia_css_process_t *process)
{

	return process;
}
#endif

int ia_css_process_set_cell(
	ia_css_process_t					*process,
	const vied_nci_cell_ID_t				cell_id)
{
	int	retval = -1;
	vied_nci_resource_bitmap_t		bit_mask;
	vied_nci_resource_bitmap_t		resource_bitmap;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_set_cell(): enter:\n");

	verifexit(process != NULL);

	parent = ia_css_process_get_parent(process);

	verifexit(parent != NULL);

	parent_state = ia_css_process_group_get_state(parent);
	state = ia_css_process_get_state(process);

/* Some programs are mapped on a fixed cell,
 * when the process group is created
 */
	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		(parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
		(parent_state == IA_CSS_PROCESS_GROUP_CREATED) ||
		/* If the process group has already been created, but no VP cell
		 * has been assigned to this process (i.e. not fixed in
		 * manifest), then we need to set the cell of this process
		 * while its parent state is READY (the ready state is set at
		 * the end of ia_css_process_group_create)
		 */
		(parent_state == IA_CSS_PROCESS_GROUP_READY)));
	verifexit(state == IA_CSS_PROCESS_READY);

/* Some programs are mapped on a fixed cell, thus check is not secure,
 * but it will detect a preset, the process manager will do the secure check
 */
	verifexit(ia_css_process_get_cell(process) ==
		  VIED_NCI_N_CELL_ID);

	bit_mask = vied_nci_cell_bit_mask(cell_id);
	resource_bitmap = ia_css_process_group_get_resource_bitmap(parent);

	verifexit(bit_mask != 0);
	verifexit(vied_nci_is_bitmap_clear(bit_mask, resource_bitmap));

	ia_css_process_cells_clear(process);
	ia_css_process_cells_set_cell(process, 0, cell_id);

	resource_bitmap = vied_nci_bitmap_set(resource_bitmap, bit_mask);

	retval = ia_css_process_group_set_resource_bitmap(
			parent, resource_bitmap);
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_set_cell invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_set_cell failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_clear_cell(
	ia_css_process_t *process)
{
	int	retval = -1;
	vied_nci_cell_ID_t				cell_id;
	ia_css_process_group_t			*parent;
	vied_nci_resource_bitmap_t		resource_bitmap;
	vied_nci_resource_bitmap_t		bit_mask;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_clear_cell(): enter:\n");
	verifexit(process != NULL);

	cell_id = ia_css_process_get_cell(process);
	parent = ia_css_process_get_parent(process);

	verifexit(parent != NULL);

	parent_state = ia_css_process_group_get_state(parent);
	state = ia_css_process_get_state(process);

	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED)
		   || (parent_state == IA_CSS_PROCESS_GROUP_STARTED)));
	verifexit(state == IA_CSS_PROCESS_READY);

	bit_mask = vied_nci_cell_bit_mask(cell_id);
	resource_bitmap = ia_css_process_group_get_resource_bitmap(parent);

	verifexit(bit_mask != 0);
	verifexit(vied_nci_is_bitmap_set(bit_mask, resource_bitmap));

	ia_css_process_cells_clear(process);

	resource_bitmap = vied_nci_bitmap_clear(resource_bitmap, bit_mask);

	retval = ia_css_process_group_set_resource_bitmap(
			parent, resource_bitmap);
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_clear_cell invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_clear_cell failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_set_int_mem(
	ia_css_process_t				*process,
	const vied_nci_mem_type_ID_t			mem_type_id,
	const vied_nci_resource_size_t			offset)
{
	int	retval = -1;
	ia_css_process_group_t	*parent;
	vied_nci_cell_ID_t	cell_id;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t	state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_int_mem(): enter:\n");

	verifexit(process != NULL);
	verifexit(mem_type_id < VIED_NCI_N_MEM_TYPE_ID);

	parent = ia_css_process_get_parent(process);
	cell_id = ia_css_process_get_cell(process);

	parent_state = ia_css_process_group_get_state(parent);
	state = ia_css_process_get_state(process);

	/* TODO : separate process group start and run from
	*	  process_group_exec_cmd()
	*/
	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
	verifexit(state == IA_CSS_PROCESS_READY);

	if (vied_nci_is_cell_mem_of_type(cell_id, mem_type_id, mem_type_id)) {
		vied_nci_mem_ID_t mem_id =
			vied_nci_cell_get_mem(cell_id, mem_type_id);

			process->int_mem_id[mem_type_id] = mem_id;
			process->int_mem_offset[mem_type_id] = offset;
			retval = 0;
	}
EXIT:
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_set_int_mem failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_clear_int_mem(
	ia_css_process_t *process,
	const vied_nci_mem_type_ID_t mem_type_id)
{
	int	retval = -1;
	uint16_t	mem_index;
	ia_css_process_group_t	*parent;
	vied_nci_cell_ID_t	cell_id;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t	state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_clear_int_mem(): enter:\n");

	verifexit(process != NULL);
	verifexit(mem_type_id < VIED_NCI_N_MEM_TYPE_ID);

	parent = ia_css_process_get_parent(process);
	cell_id = ia_css_process_get_cell(process);

	/* We should have a check on NULL != parent but it parent is NULL
	 * ia_css_process_group_get_state will return
	 * IA_CSS_N_PROCESS_GROUP_STATES so it will be filtered anyway later.
	*/

	/* verifexit(parent != NULL); */

	parent_state = ia_css_process_group_get_state(parent);
	state = ia_css_process_get_state(process);

	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED)
		   || (parent_state == IA_CSS_PROCESS_GROUP_STARTED)));
	verifexit(state == IA_CSS_PROCESS_READY);

/* We could just clear the field, but lets check the state for
 * consistency first
 */
	for (mem_index = 0; mem_index < (int)VIED_NCI_N_MEM_TYPE_ID;
	     mem_index++) {
		if (vied_nci_is_cell_mem_of_type(
			cell_id, mem_index, mem_type_id)) {
			vied_nci_mem_ID_t mem_id =
				vied_nci_cell_get_mem(cell_id, mem_index);
			int mem_of_type;

			mem_of_type =
				vied_nci_is_mem_of_type(mem_id, mem_type_id);

			assert(mem_of_type);
			assert((process->int_mem_id[mem_type_id] == mem_id) ||
				(process->int_mem_id[mem_type_id] ==
				VIED_NCI_N_MEM_ID));
			process->int_mem_id[mem_type_id] = VIED_NCI_N_MEM_ID;
			process->int_mem_offset[mem_type_id] =
				IA_CSS_PROCESS_INVALID_OFFSET;
			retval = 0;
		}
	}

EXIT:
	if (NULL == process || mem_type_id >= VIED_NCI_N_MEM_TYPE_ID) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_clear_int_mem invalid argument\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_clear_int_mem failed (%i)\n", retval);
	}
return retval;
}

int ia_css_process_set_ext_mem(
	ia_css_process_t *process,
	const vied_nci_mem_ID_t mem_id,
	const vied_nci_resource_size_t offset)
{
	int	retval = -1;
	ia_css_process_group_t	*parent;
	vied_nci_cell_ID_t	cell_id;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t	state;
	vied_nci_mem_type_ID_t mem_type_id;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_ext_mem(): enter:\n");

	verifexit(process != NULL);

	parent = ia_css_process_get_parent(process);
	cell_id = ia_css_process_get_cell(process);

	/* We should have a check on NULL != parent but it parent is NULL
	 * ia_css_process_group_get_state will return
	 * IA_CSS_N_PROCESS_GROUP_STATES so it will be filtered anyway later.
	*/

	/* verifexit(parent != NULL); */

	parent_state = ia_css_process_group_get_state(parent);
	state = ia_css_process_get_state(process);

	/* TODO : separate process group start and run from
	*	  process_group_exec_cmd()
	*/
	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
	verifexit(state == IA_CSS_PROCESS_READY);

	/* Check that the memory actually exists, "vied_nci_has_cell_mem_of_id()"
	* will return false on error
	*/

	mem_type_id = vied_nci_mem_get_type(mem_id);
	if (((!vied_nci_has_cell_mem_of_id(cell_id, mem_id) &&
		(mem_type_id != VIED_NCI_PMEM_TYPE_ID))
		|| vied_nci_mem_is_ext_type(mem_type_id)) &&
		(mem_id < VIED_NCI_N_MEM_ID)) {

		verifexit(mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID);
		process->ext_mem_id[mem_type_id] = mem_id;
		process->ext_mem_offset[mem_type_id] = offset;
		retval = 0;
	}

EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_set_ext_mem invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_set_ext_mem failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_clear_ext_mem(
	ia_css_process_t *process,
	const vied_nci_mem_type_ID_t mem_type_id)
{
	int	retval = -1;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_clear_ext_mem(): enter:\n");

	verifexit(process != NULL);
	verifexit(mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID);

	parent = ia_css_process_get_parent(process);
	state = ia_css_process_get_state(process);

	verifexit(parent != NULL);
	verifexit(state == IA_CSS_PROCESS_READY);

	parent_state = ia_css_process_group_get_state(parent);

	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_STARTED)));

	process->ext_mem_id[mem_type_id] = VIED_NCI_N_MEM_ID;
	process->ext_mem_offset[mem_type_id] = IA_CSS_PROCESS_INVALID_OFFSET;

	retval = 0;
EXIT:
	if (NULL == process || mem_type_id >= VIED_NCI_N_DATA_MEM_TYPE_ID) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_clear_ext_mem invalid argument\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_clear_ext_mem failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_set_cells_bitmap(
	ia_css_process_t *process,
	const vied_nci_resource_bitmap_t bitmap)
{
	int	retval = -1;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;
	int array_index = 0;
	int bit_index;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_cells_bitmap(): enter:\n");

	verifexit(process != NULL);
	parent = ia_css_process_get_parent(process);
	state = ia_css_process_get_state(process);

	parent_state = ia_css_process_group_get_state(parent);

	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		(parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
		(parent_state == IA_CSS_PROCESS_GROUP_CREATED) ||
		(parent_state == IA_CSS_PROCESS_GROUP_READY)));
	verifexit(state == IA_CSS_PROCESS_READY);

	for (bit_index = 0; bit_index < VIED_NCI_N_CELL_ID; bit_index++) {
		if (vied_nci_is_bit_set_in_bitmap(bitmap, bit_index)) {
			verifexit(array_index < IA_CSS_PROCESS_MAX_CELLS);
			ia_css_process_cells_set_cell(process,
				array_index, (vied_nci_cell_ID_t)bit_index);
			array_index++;
		}
	}
	for (; array_index < IA_CSS_PROCESS_MAX_CELLS; array_index++) {
		ia_css_process_cells_set_cell(process,
			array_index, VIED_NCI_N_CELL_ID);
	}

	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_set_cells_bitmap invalid argument\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_set_cells_bitmap failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_set_dev_chn(
	ia_css_process_t *process,
	const vied_nci_dev_chn_ID_t dev_chn_id,
	const vied_nci_resource_size_t offset)
{
	int	retval = -1;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_dev_chn(): enter:\n");

	verifexit(process != NULL);
	verifexit(dev_chn_id <= VIED_NCI_N_DEV_CHN_ID);

	parent = ia_css_process_get_parent(process);
	state = ia_css_process_get_state(process);

	parent_state = ia_css_process_group_get_state(parent);

	/* TODO : separate process group start and run from
	*	  process_group_exec_cmd()
	*/
	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
	verifexit(state == IA_CSS_PROCESS_READY);

	process->dev_chn_offset[dev_chn_id] = offset;

	retval = 0;
EXIT:
	if (NULL == process || dev_chn_id >= VIED_NCI_N_DEV_CHN_ID) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_set_dev_chn invalid argument\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
					"ia_css_process_set_dev_chn invalid argument\n");
	}
	return retval;
}

int ia_css_process_set_dfm_port_bitmap(
	ia_css_process_t                 *process,
	const vied_nci_dev_dfm_id_t      dfm_dev_id,
	const vied_nci_resource_bitmap_t bitmap)
{
	int	retval = -1;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_dfm_port(): enter:\n");

	verifexit(process != NULL);

	parent = ia_css_process_get_parent(process);
	state = ia_css_process_get_state(process);

	parent_state = ia_css_process_group_get_state(parent);

	/* TODO : separate process group start and run from
	*	  process_group_exec_cmd()
	*/
	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
	verifexit(state == IA_CSS_PROCESS_READY);

#if (VIED_NCI_N_DEV_DFM_ID > 0)
	verifexit(dfm_dev_id <= VIED_NCI_N_DEV_DFM_ID);
	process->dfm_port_bitmap[dfm_dev_id] = bitmap;
#else
	(void)bitmap;
	(void)dfm_dev_id;
#endif
	retval = 0;
EXIT:
	if (retval != 0) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_set_dfm_port invalid argument\n");
	}
	return retval;
}

int ia_css_process_set_dfm_active_port_bitmap(
	ia_css_process_t                 *process,
	const vied_nci_dev_dfm_id_t      dfm_dev_id,
	const vied_nci_resource_bitmap_t bitmap)
{
	int	retval = -1;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_dfm_active_port_bitmap(): enter:\n");

	verifexit(process != NULL);

	parent = ia_css_process_get_parent(process);
	state = ia_css_process_get_state(process);

	parent_state = ia_css_process_group_get_state(parent);

	/* TODO : separate process group start and run from
	*	  process_group_exec_cmd()
	*/
	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
		   (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
	verifexit(state == IA_CSS_PROCESS_READY);
#if (VIED_NCI_N_DEV_DFM_ID > 0)
	verifexit(dfm_dev_id <= VIED_NCI_N_DEV_DFM_ID);
	process->dfm_active_port_bitmap[dfm_dev_id] = bitmap;
#else
	(void)bitmap;
	(void)dfm_dev_id;
#endif
	retval = 0;
EXIT:
	if (retval != 0) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_set_dfm_active_port_bitmap invalid argument\n");
	}
	return retval;
}

int ia_css_process_clear_dev_chn(
	ia_css_process_t *process,
	const vied_nci_dev_chn_ID_t dev_chn_id)
{
	int	retval = -1;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_clear_dev_chn(): enter:\n");

	verifexit(process != NULL);

	parent = ia_css_process_get_parent(process);

	/* We should have a check on NULL != parent but it parent is NULL
	 * ia_css_process_group_get_state will return
	 * IA_CSS_N_PROCESS_GROUP_STATES so it will be filtered anyway later.
	*/

	/* verifexit(parent != NULL); */

	parent_state = ia_css_process_group_get_state(parent);
	state = ia_css_process_get_state(process);

	verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED)
		   || (parent_state == IA_CSS_PROCESS_GROUP_STARTED)));
	verifexit(state == IA_CSS_PROCESS_READY);

	verifexit(dev_chn_id <= VIED_NCI_N_DEV_CHN_ID);

	process->dev_chn_offset[dev_chn_id] = IA_CSS_PROCESS_INVALID_OFFSET;

	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
		     "ia_css_process_clear_dev_chn invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_clear_dev_chn failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_clear_all(
	ia_css_process_t *process)
{
	int	retval = -1;
	ia_css_process_group_t			*parent;
	ia_css_process_group_state_t	parent_state;
	ia_css_process_state_t			state;
	int	mem_index;
	int	dev_chn_index;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_clear_all(): enter:\n");

	verifexit(process != NULL);

	parent = ia_css_process_get_parent(process);
	state = ia_css_process_get_state(process);

	/* We should have a check on NULL != parent but it parent is NULL
	 * ia_css_process_group_get_state will return
	 * IA_CSS_N_PROCESS_GROUP_STATES so it will be filtered anyway later.
	*/

	/* verifexit(parent != NULL); */

	parent_state = ia_css_process_group_get_state(parent);

/* Resource clear can only be called in excluded states contrary to set */
	verifexit((parent_state != IA_CSS_PROCESS_GROUP_RUNNING) ||
		   (parent_state == IA_CSS_N_PROCESS_GROUP_STATES));
	verifexit((state == IA_CSS_PROCESS_CREATED) ||
		  (state == IA_CSS_PROCESS_READY));

	for (dev_chn_index = 0; dev_chn_index < VIED_NCI_N_DEV_CHN_ID;
		dev_chn_index++) {
		process->dev_chn_offset[dev_chn_index] =
			IA_CSS_PROCESS_INVALID_OFFSET;
	}
/* No difference whether a cell_id has been set or not, clear all */
	for (mem_index = 0; mem_index < VIED_NCI_N_DATA_MEM_TYPE_ID;
		mem_index++) {
		process->ext_mem_id[mem_index] = VIED_NCI_N_MEM_ID;
		process->ext_mem_offset[mem_index] =
			IA_CSS_PROCESS_INVALID_OFFSET;
	}
	for (mem_index = 0; mem_index < VIED_NCI_N_MEM_TYPE_ID; mem_index++) {
		process->int_mem_id[mem_index] = VIED_NCI_N_MEM_ID;
		process->int_mem_offset[mem_index] =
			IA_CSS_PROCESS_INVALID_OFFSET;
	}

	ia_css_process_cells_clear(process);

	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_clear_all invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_clear_all failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_acquire(
	ia_css_process_t *process)
{
	int	retval = -1;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_acquire(): enter:\n");

	verifexit(process != NULL);

	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_acquire invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_acquire failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_release(
	ia_css_process_t *process)
{
	int	retval = -1;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO,
		"ia_css_process_release(): enter:\n");

	verifexit(process != NULL);

	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_t invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_release failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_print(const ia_css_process_t *process, void *fid)
{
	int		retval = -1;
	int		i, dev_chn_index;
	uint16_t mem_index;
	uint8_t	cell_dependency_count, terminal_dependency_count;
	vied_nci_cell_ID_t	cell_id = ia_css_process_get_cell(process);
	NOT_USED(fid);

	IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
		"ia_css_process_print(process %p): enter:\n", process);

	verifexit(process != NULL);

	IA_CSS_TRACE_6(PSYSAPI_DYNAMIC, INFO,
	"\tprocess %p, sizeof %d, programID %d, state %d, parent %p, cell %d\n",
		process,
		(int)ia_css_process_get_size(process),
		(int)ia_css_process_get_program_ID(process),
		(int)ia_css_process_get_state(process),
		(void *)ia_css_process_get_parent(process),
		(int)ia_css_process_get_cell(process));

	for (mem_index = 0; mem_index < (int)VIED_NCI_N_MEM_TYPE_ID;
		mem_index++) {
		vied_nci_mem_ID_t mem_id =
			(vied_nci_mem_ID_t)(process->int_mem_id[mem_index]);
		if (cell_id == VIED_NCI_N_CELL_ID) {
			verifexit(mem_id == VIED_NCI_N_MEM_ID);
			continue;
		}
		verifexit(((mem_id == vied_nci_cell_get_mem(cell_id, mem_index))
			|| (mem_id == VIED_NCI_N_MEM_ID)));

		IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
			"\tinternal index %d, type %d, id %d offset 0x%x\n",
			mem_index,
			(int)vied_nci_cell_get_mem_type(cell_id, mem_index),
			(int)mem_id,
			process->int_mem_offset[mem_index]);
	}

	for (mem_index = 0; mem_index < (int)VIED_NCI_N_DATA_MEM_TYPE_ID;
		mem_index++) {
		vied_nci_mem_ID_t mem_id =
			(vied_nci_mem_ID_t)(process->ext_mem_id[mem_index]);
		/* TODO: in case of an cells_bitmap = [],
		 * vied_nci_cell_get_mem_type will return a wrong result.
		 */
		IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
			"\texternal index %d, type %d, id %d offset 0x%x\n",
			mem_index,
			(int)vied_nci_cell_get_mem_type(cell_id, mem_index),
			(int)mem_id,
			process->ext_mem_offset[mem_index]);
		NOT_USED(mem_id);
	}
	for (dev_chn_index = 0; dev_chn_index < (int)VIED_NCI_N_DEV_CHN_ID;
		dev_chn_index++) {
		IA_CSS_TRACE_3(PSYSAPI_DYNAMIC, INFO,
			"\tdevice channel index %d, type %d, offset 0x%x\n",
			dev_chn_index,
			(int)dev_chn_index,
			process->dev_chn_offset[dev_chn_index]);
	}
#if HAS_DFM
	for (dev_chn_index = 0; dev_chn_index < (int)VIED_NCI_N_DEV_DFM_ID;
		dev_chn_index++) {
		IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
			"\tdfm device index %d, type %d, bitmap 0x%x active_ports_bitmap 0x%x\n",
			dev_chn_index, dev_chn_index,
			process->dfm_port_bitmap[dev_chn_index],
			process->dfm_active_port_bitmap[dev_chn_index]);
	}
#endif

	for (i = 0; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
		IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO,
			"\tcells[%d] = 0x%x\n",
			i, ia_css_process_cells_get_cell(process, i));
	}

	cell_dependency_count =
		ia_css_process_get_cell_dependency_count(process);
	if (cell_dependency_count == 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
			"\tcell_dependencies[%d] {};\n", cell_dependency_count);
	} else {
		vied_nci_resource_id_t cell_dependency;

		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
			"\tcell_dependencies[%d] {", cell_dependency_count);
		for (i = 0; i < (int)cell_dependency_count - 1; i++) {
			cell_dependency =
				ia_css_process_get_cell_dependency(process, i);
			IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
				"%4d, ", cell_dependency);
		}
		cell_dependency =
			ia_css_process_get_cell_dependency(process, i);
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
			"%4d}\n", cell_dependency);
		(void)cell_dependency;
	}

	terminal_dependency_count =
		ia_css_process_get_terminal_dependency_count(process);
	if (terminal_dependency_count == 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
			"\tterminal_dependencies[%d] {};\n",
			terminal_dependency_count);
	} else {
		uint8_t terminal_dependency;

		terminal_dependency_count =
			ia_css_process_get_terminal_dependency_count(process);
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
			"\tterminal_dependencies[%d] {",
			terminal_dependency_count);
		for (i = 0; i < (int)terminal_dependency_count - 1; i++) {
			terminal_dependency =
			     ia_css_process_get_terminal_dependency(process, i);
			IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
				"%4d, ", terminal_dependency);
		}
		terminal_dependency =
			ia_css_process_get_terminal_dependency(process, i);
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
			"%4d}\n", terminal_dependency);
		(void)terminal_dependency;
	}

	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_print invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_print failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_set_parent(
	ia_css_process_t					*process,
	ia_css_process_group_t					*parent)
{
	int	retval = -1;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_parent(): enter:\n");

	verifexit(process != NULL);
	verifexit(parent != NULL);

	process->parent_offset = (uint16_t) ((char *)parent - (char *)process);
	retval = 0;
EXIT:
	if (NULL == process || NULL == parent) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_set_parent invalid argument\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_set_parent failed (%i)\n", retval);
	}
	return retval;
}

int ia_css_process_set_cell_dependency(
	const ia_css_process_t					*process,
	const unsigned int					dep_index,
	const vied_nci_resource_id_t				id)
{
	int retval = -1;
	uint8_t *process_dep_ptr;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_set_cell_dependency(): enter:\n");
	verifexit(process != NULL);

	process_dep_ptr =
		(uint8_t *)process + process->cell_dependencies_offset +
			   dep_index*sizeof(vied_nci_resource_id_t);


	*process_dep_ptr = id;
	retval = 0;
EXIT:
	return retval;
}

int ia_css_process_set_terminal_dependency(
	const ia_css_process_t				*process,
	const unsigned int				dep_index,
	const vied_nci_resource_id_t		id)
{
	int retval = -1;
	uint8_t *terminal_dep_ptr;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_set_terminal_dependency(): enter:\n");
	verifexit(process != NULL);
	verifexit(ia_css_process_get_terminal_dependency_count(process) > dep_index);

	terminal_dep_ptr =
		(uint8_t *)process + process->terminal_dependencies_offset +
			   dep_index*sizeof(uint8_t);

	*terminal_dep_ptr = id;
	retval = 0;
EXIT:
	return retval;
}

int ia_css_process_cmd(
	ia_css_process_t					*process,
	const ia_css_process_cmd_t				cmd)
{
	int	retval = -1;
	ia_css_process_state_t	state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO, "ia_css_process_cmd(): enter:\n");

	verifexit(process != NULL);

	state = ia_css_process_get_state(process);

	verifexit(state != IA_CSS_PROCESS_ERROR);
	verifexit(state < IA_CSS_N_PROCESS_STATES);

	switch (cmd) {
	case IA_CSS_PROCESS_CMD_NOP:
		break;
	case IA_CSS_PROCESS_CMD_ACQUIRE:
		verifexit(state == IA_CSS_PROCESS_READY);
		break;
	case IA_CSS_PROCESS_CMD_RELEASE:
		verifexit(state == IA_CSS_PROCESS_READY);
		break;
	case IA_CSS_PROCESS_CMD_START:
		verifexit((state == IA_CSS_PROCESS_READY)
			  || (state == IA_CSS_PROCESS_STOPPED));
		process->state = IA_CSS_PROCESS_STARTED;
		break;
	case IA_CSS_PROCESS_CMD_LOAD:
		verifexit(state == IA_CSS_PROCESS_STARTED);
		process->state = IA_CSS_PROCESS_RUNNING;
		break;
	case IA_CSS_PROCESS_CMD_STOP:
		verifexit((state == IA_CSS_PROCESS_RUNNING)
			  || (state == IA_CSS_PROCESS_SUSPENDED));
		process->state = IA_CSS_PROCESS_STOPPED;
		break;
	case IA_CSS_PROCESS_CMD_SUSPEND:
		verifexit(state == IA_CSS_PROCESS_RUNNING);
		process->state = IA_CSS_PROCESS_SUSPENDED;
		break;
	case IA_CSS_PROCESS_CMD_RESUME:
		verifexit(state == IA_CSS_PROCESS_SUSPENDED);
		process->state = IA_CSS_PROCESS_RUNNING;
		break;
	case IA_CSS_N_PROCESS_CMDS:	/* Fall through */
	default:
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_cmd invalid cmd (0x%x)\n", cmd);
		goto EXIT;
	}
	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_cmd invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_cmd failed (%i)\n", retval);
	}
	return retval;
}

