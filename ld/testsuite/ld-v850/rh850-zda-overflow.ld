SECTIONS
{
  zfar = 0x8000;
  . = 0x10000;
  .text : { *(.text); }
  /DISCARD/ : { *(*); }
}
