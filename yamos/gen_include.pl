#! /bin/perl
#
# Extends include-header-files with __attribute__((packed))
# to make ixemul and os-includes ppc-compatible
#
# 07.06.98 Samuel Devulder: __attribute((aligned(2)) for all >2 byte elements
# 18.06.98 Holger Jakob:    It is not enough to use a modified
#                           exec/types.h only :(
# 21.05.2000 Emmanuel Lesueur: don't put __attribute__((aligned(2)) for structure
#                              elements that are on correct boudaries.
#
require 5.002;

# structs that have a length non multiple of 4:

%oddstructs=(
	AChain => 1,
	AnchorPath => 1,
	AnimComp => 1,
	AnimOb => 1,
	AppMessage => 1,
	AvailFonts => 1,
	AvailFontsHeader => 1,
	CDInfo => 1,
	CIA => 1,
	ClipboardUnitPartial => 1,
	ClockData => 1,
	ColorRegister => 1,
	CopIns => 1,
	CopList => 1,
	Custom => 1,
	DTSpecialInfo => 1,
	DataType => 1,
	DateTime => 1,
	Device => 1,
	DeviceTData => 1,
	DiscResourceUnit => 1,
	DiskFontHeader => 1,
	DiskObject => 1,
	DosInfo => 1,
	DosLibrary => 1,
	DrawInfo => 1,
	DrawerData => 1,
	FileSysEntry => 1,
	GadgetInfo => 1,
	GlyphWidthEntry => 1,
	IEPointerTablet => 1,
	IODRPReq => 1,
	IOExtPar => 1,
	IOExtSer => 1,
	IOPrtCmdReq => 1,
	InputEvent => 1,
	Interrupt => 1,
	Isrvstr => 1,
	KeyMapNode => 1,
	Layer_Info => 1,
	Library => 1,
	List => 1,
	Menu => 1,
	Node => 1,
	NotifyMessage => 1,
	PrefHeader => 1,
	PrinterData => 1,
	PrinterExtendedData => 1,
	PrinterGfxPrefs => 1,
	PrinterSegment => 1,
	PrtInfo => 1,
	PubScreenNode => 1,
	RGBTable => 1,
	Resident => 1,
	RexxTask => 1,
	SCSICmd => 1,
	SatisfyMsg => 1,
	SerialPrefs => 1,
	SignalSemaphore => 1,
	SpecialMonitor => 1,
	TAvailFonts => 1,
	TOCEntry => 1,
	TOCSummary => 1,
	ToolNode => 1,
	Unit => 1,
	View => 1,
	ViewPortExtra => 1,
	bltnode => 1,
	cprlist => 1,
);

undef $/;

die "no include" if not $include=@ARGV[0];
print STDERR "Copying $include ... ";

open(INP,"<$include");
$source=<INP>;

# Check for additional patches here...
# user.h, (screens.h), setjmp.h...

#if( $include=~ /include\/user.h$/i ) { $source=~ s/(\W)jmp_buf(\W)/$1jmp_buf_m68k$2/g; }
#if( $include=~ /include\/setjmp.h$/i ) {
#  $source=~ s/(#define\s+_JBLEN)(\s+\d+)/$1_M68K $2\n$1\t\t26+17*2/;
#  $source=~ s/(typedef\s+int\s+sigjmp_buf)(.*)(_JBLEN)(.*)/$1_m68k$2$3_M68K$4\n$1$2$3$4/;
#  $source=~ s/(typedef\s+int\s+jmp_buf)(.*)(_JBLEN)(.*)/$1_m68k$2$3_M68K$4\n$1$2$3$4/;
#}
if( $include=~ /include\/sys\/syscall.h$/i ) { $source=~ s/#ifndef\s+?_KERNEL(.*?)#endif//s; }

#
#

$source=~ s/\/\*.*?\*\///sg;  # Sorry, no comments
$source=~ s/^(\s*)struct((.|\n)*?)({(.|\n)*?});/&ins_packed_struct($2,$4)/meg;
# ToDo: same for typdef
#$source=~ s/^typedef((.|\n)*?)((\w|\n)*?);/&ins_packed_typedef($1,$3)/meg;
print $source;

close(INP);
print STDERR "Applied ";
print STDERR ($source=~ s/__attribute/__attribute/g) || "no";
print STDERR " patches.\n";

$alignment=0;
$max_align=0;

sub ins_packed_struct
{
	local ($name,$text)=@_;
	local ($return);

	$alignment=0;
	$max_align=0;

#       $text=~ s/(LONG|struct)([^;])/$1$2 __attribute__((aligned(2))) /g;

	$return="struct".$name.$text." __attribute__((packed));";

#FIXME: /* ; */ is not recogniced and 2-word types(eg. unsigned int) as well
#FIXED!?
	$return=~ s/^(\s*)(\w*)(\s*)([a-zA-Z0-9_]*)(.*?);/&ins_align($1,$2,$3,$4,$5)/ge;

	if($max_align>1 && $alignment>0 && ($alignment&1)!=0) {
	    $return=~ s/(.*)}(\s*__attribute__\(\(packed\)\).*)/$1\tchar __pad__;\n}$2/;
	}
	return $return;
}
sub ins_packed_typedef
{
	local ($text,$name)=@_;
	local ($return);


#       $text=~ s/(LONG|struct)([^;])/$1$2 __attribute__((aligned(2))) /g;

#       $return="struct".$name.$text." __attribute__((packed));";

#FIXME: /* ; */ is not recogniced and 2-word types(eg. unsigned int) as well
#FIXED!?
#       $return=~ s/^(\s*)(\w*)(\s*)([a-zA-Z_]*)(.*?);/&ins_align($1,$2,$3,$4,$5)/ge;

	return "typedef $name;";
}

sub ins_align
{
	local ($space,$type,$space2,$type2,$part1)=@_;
	local ($size,$x);
	$size=0;

	if ( $part1=~ /^\s*\*/ ) {
	    $size=4;
	    if ( $alignment!= 0 ) {
		if ( $part1=~ /^\s*(\**)(\w*)(.*)/ ) {
		    $part1=$1." __attribute__((aligned(2))) ".$2.$3;
		}
	    }
	} elsif( $type=~ /^(BYTE|UBYTE|BYTEBITS|TEXT|char)$/ ) {
	    $size=1;
	} elsif( $type=~ /^(WORD|UWORD|SHORT|USHORT|BOOL|COUNT|UCOUNT|WORDBITS|short)$/ ) {
	    $size=2;
	} elsif( $type=~ /^struct$/ ) {
	    if ($alignment != 0) {
		$type2=$type2." __attribute__((aligned(2)))";
	    }
	    if( exists $oddstructs{$type2} ) {
		$size=2;
	    } else {
		$size=4;
	    }
	} elsif( $alignment!=0 ) {
	    if( $type=~ /^([AC]PTR|STRPTR|LONG|LONGBITS|ULONG|FLOAT|DOUBLE)$/ ) {
		$type=$type." __attribute__((aligned(2)))";
		$size=4;
	    } elsif( $type=~ /^unsigned$/ ) {
		$type2=$type2." __attribute__((aligned(2)))";
		$size=4;
	    } elsif( $type=~ /^(int|long)$/ ) {
		$type=$type." __attribute__((aligned(2)))";
		$size=4;
	    } elsif( $part1=~ /^(\*|\(\*)/ ) {
		$type=$type." __attribute__((aligned(2)))";
		$size=4;
	    }
	}
	if( $alignment==1 && $size>1 ) {
	    $alignment=2;
	} elsif( $alignment==3 && $size>1) {
	    $alignment=0;
	}
	if( $size!=0 && $alignment!=-1 ) {
	    $x=$part1;
	    while ($alignment!=-1 && $x=~/\[/) {
		if( $x=~ /\[(\d*)\](.*)/ ) {
		    $size*=$1;
		    $x=$2;
		} else {
		    $alignment=-1;
		}
	    }
	    if( $alignment!=-1 ) {
		$alignment=($alignment+$size)&3;
	    }
	}
	if( $size > $max_align) {
	    $max_align = $size;
	}
#        return "/* ".$alignment.",".$max_align." */ ".$space.$type.$space2.$type2.$part1.";";
	return $space.$type.$space2.$type2.$part1.";";
}
