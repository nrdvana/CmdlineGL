#! /usr/bin/perl
use strict;
use warnings;
open(STDERR, '>&STDOUT');

# Run a command like 'make', but capture the output, and show the output if it fails.
sub run {
	my ($out, $cmd_fh);
	local $/= undef;
	unless (open($cmd_fh, '-|', @_) and do { $out= <$cmd_fh>; close $cmd_fh }) {
		$|= 1;
		print $out;
		my $exitreason= $? == -1? "exec: $!" : $? & 0x7F? "died on signal ".($?&0x7F) : "exited with code ".($?>>8);
		die "Command failed: \"".join('" "', @_)."\": $exitreason\n";
	}
	$out;
}

my $proj_root= $ENV{PROJROOT};
-d "$proj_root/src" and -d "$proj_root/script"
	or die "Incorrect PROJROOT \"$proj_root\"\n";

# Verify we have all changes checked in

my $uncommitted= run('git',"--git-dir=$proj_root/.git","--work-tree=$proj_root",'status','--porcelain');
$uncommitted =~ /\S/
	and die "Uncommitted git changes!";

# Git HEAD should be tagged same as Changes file

chomp(my $git_head= run('git','log','-n','1','--format=format:%H%d'));
$git_head =~ /tag: v([^)]+)/ or die "HEAD lacks a tag: \"$git_head\"\n";
my $git_ver= $1;
open(my $changes_fh, '<', "$proj_root/Changes") or die "open(Changes): $!";
my $changes_ver_line= <$changes_fh>;
$changes_ver_line =~ /Version ([^. ]+\.[^. ]+\.[^. ]+) / or die "Unexpected format in Changes: \"$changes_ver_line\"\n";
my $changes_ver= $1;

$git_ver eq $changes_ver or die "Version Mismatch between git ($git_ver) and Changes ($changes_ver)\n";

# Clone project into a temporary directory

run('rm', '-rf', "$proj_root/dist/next");
mkdir "$proj_root/dist";
mkdir "$proj_root/dist/next";
my $dest_dir= "$proj_root/dist/next";

my $out;
$out= run('git',"--work-tree=$dest_dir","--git-dir=$proj_root/.git",'checkout','.');

# Remove debug and dev configuration that we added by default

run('sed', '-i', '-e', 's/--enable-debug/--disable-debug/', "$dest_dir/configure");

# Test that we can compile and build it

run('make','-C',$dest_dir,'all');
run('make','-C',"$dest_dir/build",'test');

# If that worked, wipe the build dir and zip it!

my $distname= "CmdlineGL-$changes_ver";
run('rm', '-rf', "$dest_dir/build");
rename $dest_dir, "$proj_root/dist/$distname" or die "rename to \"$proj_root/dist/$distname\" failed: $!";
run('tar', '-C', "$proj_root/dist", '-cjf', "$proj_root/dist/$distname.tar.bz2", $distname);

print "\nBuilt $proj_root/dist/$distname.tar.bz2\n\n";

