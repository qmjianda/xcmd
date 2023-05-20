ENTRY(main)
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K

}

SECTIONS
{
	.text :
	{
		KEEP(*(.isr_vector))
 		*(.text)
 		*(.text.*)
		*(.rodata)
		*(.rodata.*)
	} >FLASH

	.xcmd_cmd :
	{
		_xcmd_cmd_list_start = .;
		KEEP(*(_xcmd_cmd_list))
		_xcmd_cmd_list_end = .;
	} > FLASH

	.xcmd_key : 
	{
		_xcmd_key_list_start = .;
		KEEP(*(_xcmd_key_list))
		_xcmd_key_list_end = .;
	} > FLASH

	_sidata = .;


	/* Initialized data will initially be loaded in FLASH at the end of the .text section. */
    .data : AT (_sidata) 
    {
		_sdata = .;
		*(.data)		/* Initialized data */
		_edata = .;
	} >RAM
	.bss : {
		_sbss = .;
		*(.bss)         /* Zero-filled run time allocate data memory */
		_ebss = .;
	} >RAM

	PROVIDE ( end = _ebss );
	PROVIDE ( _end = _ebss );
    _estack = ORIGIN(RAM) + LENGTH(RAM);
 }  