#!/bin/perl
#                          _   _                  _        __
#   __ _  __ _ _   _  __ _| |_(_) ___ _   _ ___  (_)_ __  / _| ___
#  / _` |/ _` | | | |/ _` | __| |/ __| | | / __| | | '_ \| |_ / _ \
# | (_| | (_| | |_| | (_| | |_| | (__| |_| \__ \_| | | | |  _| (_) |
#  \__,_|\__, |\__,_|\__,_|\__|_|\___|\__,_|___(_)_|_| |_|_|  \___/
#           |_|
#
# Copyright (c) 2012, All Right Reserved, http://aquaticus.info
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

# Converts font definition file to "C" tables
# See help for more info 

sub HELP_MESSAGE
{
	print  <<'HELP';
fontconvert.pl [OPTIONS] input_file output_file
Convert text font file format into C tables.
(c) 2010 aquaticus.info

Options:
-b hex data table base address (0 if not passed).
-c show font data in human readable form
-t C table name (default FONTTAB)
-s data section (default .font)
-p table in progmem (__ATTR_PROGMEM__)

Format of char:
Use . (dot) and # (hash)
Font def must start with 'Code #' then optional char ascii code.
If ASCII code is not present, the next free code is used.
Text before and after ........ is ignored.
Bitmaps bigger then 8x8 will be automatically divided into separate 8x8 tiles.
Character data MUST be finished with the empty line.

Code #
.######.
.....##.
....##..
...##...
..##....
..##....
..##....
........
(empty line!!!!)
HELP

exit;
}

use Getopt::Std;
use strict;

my @fonttable;
my $ascii;
my %tile_info;
		
$ascii = 0;

our($opt_b, $opt_s, $opt_t, $opt_c, $opt_p);

getopt("bctsh");

#HELP_MESSAGE() if $#ARGV < 2;

my $in = $ARGV[0] or die("You must pass input file name as first argument");
my $out = $ARGV[1] or die("You must pass output file name as the second argument");

open FILE, "<$in" or die("Can not open input file $in. $!");


my $char_count;
$char_count = 0;

while( <FILE> )
{
	if( /Code\s*#(\d*)/ )
	{
		if( length($1) > 0 )
		{
			$ascii=$1;
		}
		
		my $width;
		my $height;
		my $BytesPerLine;
		my $linecount;
		my @imagetab;
		my $pattern;
		
		$height = 0;
		$width = 0;
		$linecount=0;
		
		#read character lines
		while(1)
		{
			my $line = <FILE>;
			if( $line =~ /([\.#]+)/ )
			{
				$pattern = $1;
			}
			else
			{
				last
			}

			if($width == 0)
			{
				$width = length($pattern);
				$BytesPerLine = $width / 8 + (($width % 8) ? 1 : 0);
			}
 
			die("Length of all lines in character bitmap must be the same. Line number $.") if( length($pattern) != $width );

			for(my $b=0; $b<$BytesPerLine; $b++)
			{
				my $str = substr($pattern, $b*8, 8 );
			
				$str =~ s/\./0/g;
				$str =~ s/#/1/g;
			
				my $byte = bin2dec( $str );
			
				#$imagetab[$height*$BytesPerLine+$b] = $byte;
				push @imagetab, $byte;
			}
			
			$height++;
		}
		
		my $byteidx = 0;
		my $rows = $height/8 + (($height%8) ? 1 : 0 );
		my $tile_counter = 0;
		
		if( length( @imagetab ) > 0 )
		{
			my $total_tiles = $rows*$BytesPerLine;
			
			for(my $row=0; $row<$rows; $row++)
			{
				for(my $col=0; $col<$BytesPerLine; $col++)				
				{
					#print("*** Col: $col, Row: $row\n");
					for(my $b=0; $b<8; $b++)
					{
						$byteidx = $row*$BytesPerLine*8 + $col+$b*$BytesPerLine;
						#$fonttable[$ascii+256*$b] = $imagetab[$byteidx];
						$fonttable[$ascii*8+$b] = $imagetab[$byteidx];
												
						#printf "Index: %d 0x%02X\n", $byteidx, $imagetab[$byteidx];
					}
					
					$tile_counter++;

					if( $total_tiles > 1 )
					{				
						$tile_info{$ascii} = "Tile $tile_counter of $total_tiles. Img size $BytesPerLine" . "x" . "$rows characters";
					}
					
					$ascii++;
				}
				
				$char_count ++;
				
			}
		}
		
	}
}

die("Too many characters in the input file. Max 256, is $char_count") if $char_count > 256;

close(FILE);

open OUTFILE, ">$out" or die("Can not open output file $in. $!");

DumpDefaultFormat();

dumpC();

close(OUTFILE);

print "Converted $char_count characters.\n";

exit;

sub DumpDefaultFormat()
{
	my $byte;
	my $baseadr;
	$baseadr = 0;
	if( defined $opt_b )
	{
		$baseadr = hex($opt_b);
	}
	
	print OUTFILE "/*\n";
	
	#printf OUTFILE "Base address 0x%04X\n\n", $baseadr;
	printf OUTFILE "DO NOT EDIT. This file was generated.\n\n", $baseadr;
	
	for(my $i=0;$i<$char_count;$i++)
	{
		my $tileinf = $tile_info{$i};
		print OUTFILE "$tileinf\n" if defined $tileinf;
		printf OUTFILE "Code #%d (0x%02X) [%c]\n", $i, $i, ($i < 32 ? 32 : $i);
		for(my $y=0;$y<8;$y++)
		{
			#$byte = $y*256 + $i;
			$byte = $y + $i*8;
			printf OUTFILE "%04X: %s (0x%02X)\n", $baseadr + $byte, dec2bin( $fonttable[$byte] ), $fonttable[$byte];
			
		}
		
		print OUTFILE "\n";
	}
	
	print OUTFILE "*/\n\n";
}

sub bin2dec 
{
    return unpack("N", pack("B32", substr("0" x 32 . shift, -32)));
}

sub dec2bin 
{
    my $str = unpack("B32", pack("N", shift));
    $str =~ s/^0+(?=\d)//;   # otherwise you'll get leading zeros
    $str = sprintf("%08s", $str );
    $str =~ s/0/\./g;
	$str =~ s/1/#/g;
	return $str;    
}

sub dumpC
{
	my $table = $opt_t || "FONTTAB";
	print OUTFILE "\nunsigned char";
	if( defined $opt_s )
	{
		print OUTFILE " __attribute__ ((section (\"$opt_s\")))";
	} 
	else
	{
		print OUTFILE "  __attribute__((__progmem__))";
	}
	#print OUTFILE " \$table[8*256]=\n{\n";
	print OUTFILE " $table"; 
	print OUTFILE "[8*$char_count]=\n{\n";
	
	for(my $i=0;$i<$char_count*8;$i++)
	{
		printf OUTFILE "0x%02X,", $fonttable[$i];
		print OUTFILE "\n" unless ($i+1) % 8;
	}
	print OUTFILE "};\n";
}
