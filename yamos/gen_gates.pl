#!/bin/perl
#
# gen_gates.pl - Copyright 2000 by E. Lesueur
#
# This program is in the public domain.
#
# Generates MorphOS gates for YAM.

undef $/;

die "No source" if not $file=@ARGV[0];

open(INP,"<$file") || die "Can't open \"".$file."\"\n";
$source = <INP>;

$source =~ s/\n(SAVEDS\s*(ASM)?\s*)(.*?)\s*\n{\s*\n(.*?)\n}\s*\n/cv_func($3,$4)/egs;

close <INP>;


sub cv_func {
    local ($decl, $body) = @_;
    local ($ret_type, $name, $params, $trap, $ret);
    local ($args, $reg, $type, $param, $params);

    if ($decl =~ /^(.*?)(\w+)\((.*)\)$/) {
	$ret_type = $1;
	$name = $2;
	$params = $3;
    } else {
	die "Strange declaration :\"".$decl."\"\n";
    }

    if ($ret_type =~ /^\s*void\s*$/) {
	$trap = "TRAP_LIBNR";
	$ret = "";
    } else {
	$trap = "TRAP_LIB";
	$ret = "return ";
    }

    $args = "";
    while ($params =~ /^\s*REG\((\w\w)\)\s*(.*?)\s*(\w+)(,(.*)$|$)/) {
	$reg = $1;
	$type = $2;
	$param = $3;
	$params = $5;
	if (length($args) != 0) {
	    $args = $args.", ";
	}
	$args = $args."(".$type.")REG_".uc($reg);
	if ($type =~ /struct\s+(\w+)/) {
	    print "struct $1;\n";
	}
    }

    $decl =~ s/REG\(\w\w\)\s*//g;

    print "$decl;\n"
	   ."static $ret_type Trampoline_$name(void)\n{\n"
	   ."   $ret$name($args);\n}\n"
	   ."const struct EmulLibEntry Gate_$name"
	   ." = { $trap, 0, (void(*)())Trampoline_$name };\n\n";

    return "";
}

