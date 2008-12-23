#!/bin/bash
###############################################################################
#
# Step by step script to 
# - fetch the remote vocabulary dump file
# - select a subset of the available nemes - possibly just one item
# - select the related ancestors and decendants to construct a context for the term
# - Output a CSV file that can be imported into Drupal taxonomy.
#
# This is specifically designed to massage and import ONLY the raw dump files provided by NCBI
# ftp://ftp.ncbi.nih.gov/pub/taxonomy/
#
# @author dman dan@coders.co.nz
# @version $Id: taxonomy-slicer.sh,v 1.1.4.2 2008/09/06 15:27:39 dman Exp $
###############################################################################

###############################################################################
# SETTINGS
# Add to this list for greater depth
ancestors="parent grandparent greatgrandparent 4parent 5parent 6parent 7parent 8parent 9parent 10parent 11parent 12parent 13parent 14parent 15parent 16parent 17parent 18parent  ";
descendants="child grandchild greatgrandchild 4child 5child";

# or keep it in the family
ancestors="parent";
descendants="child";

# Choose an ID or name pattern to focus on
# eg a list of apteryxs
pattern="Apteryx";
# or id 
pattern="^3627\t";
# or ids 8800-8999
pattern="^8[8-9][0-9][0-9]\t";

###############################################################################
# BEGIN
#

# Fetch dump if needed
#
if [ -f names.dmp ] ; then
 echo "The dump file is available already";
else
  wget wget ftp://ftp.ncbi.nih.gov/pub/taxonomy/taxdump.tar.gz
  tar -xzf taxdump.tar.gz
fi

# Select a number range to focus the taxonomy building on
#
# Fetch the selection
sed -n "/$pattern/p" names.dmp | awk 'BEGIN {FS="|"} {print $1}' | sort| uniq > subset-ids.txt
subsets=`cat subset-ids.txt`
echo $subsets


###############################################################################
# Loop the parent-finding process several times, collating the resulting IDs
#
cp subset-ids.txt subset-parent-ids.txt
for lineage in $ancestors; do
  echo "Retrieving $lineage IDs"; 
  subsetparents=;
  # scan for grandparents of the subsets
  subsetparents=`cat subset-parent-ids.txt`;
  subpattern=dummy;
  # subpattern is a big regexp containing all current IDs
  for subid in $subsetparents; do subpattern="$subpattern\|^$subid\t" ; done;
  #echo $subpattern;
  sed -n "/$subpattern/p" nodes.dmp | awk 'BEGIN {FS="|"} {print $2}' >> subset-parent-ids.txt;
  sort subset-parent-ids.txt | uniq > subset-parents.uniq; mv subset-parents.uniq subset-parent-ids.txt;
done;

# We now have a list of ids that are higer (and also probably include) our subsets

echo "IDs for several generations up are :"
cat subset-parent-ids.txt

###############################################################################
# Loop the child-finding process several times, collating the resulting IDs
# Scan to match the second column (parents), retrieving the first col (child)
#
cp subset-ids.txt subset-child-ids.txt
for lineage in $descendants ; do
  echo "Retrieving $lineage IDs"; 
  subsetparents=;
  # scan for children of the current set
  subsetset=`cat subset-child-ids.txt`;
  subpattern=dummy;
  # subpattern is a big regexp containing all current IDs
  for subid in $subsetset; do subpattern="$subpattern\|\t$subid\t" ; done;
  #echo $subpattern;
  sed -n "/$subpattern/p" nodes.dmp | awk 'BEGIN {FS="|"} {print $1}' >> subset-child-ids.txt;
  sort subset-child-ids.txt | uniq > subset-child.uniq; mv subset-child.uniq subset-child-ids.txt;
done;

echo "IDs for several generations down are :"
cat subset-child-ids.txt
# we now have a list of ids that are lower (and also probably include) our subsets


###############################################################################
# Now retrieve names of all the nodes of interest, for review
idsofinterest=`cat subset-child-ids.txt subset-parent-ids.txt | sort | uniq`

echo "Constructing a triplestore CSV describing each element of interest"

subpattern=dummy;
for subid in $idsofinterest; do subpattern="$subpattern\|^$subid\t" ; done;

sed -n "/$subpattern/p" names.dmp | grep "scientific name" | awk 'BEGIN {FS="|"} {print $1,", name ,", $2}' > subset-all-triples.csv
sed -n "/$subpattern/p" names.dmp | grep "synonym" | awk 'BEGIN {FS="|"} {print $1, ", Used for ,", $2}' >> subset-all-triples.csv
sed -n "/$subpattern/p" names.dmp | sed -n "/\tcommon name/p" | awk 'BEGIN {FS="|"} {print $1,", Definition ,Common Name:", $2}' >> subset-all-triples.csv
sed -n "/$subpattern/p" names.dmp | awk 'BEGIN {FS="|"} {print $1,", Definition ,GenBankID:", $1}' | uniq >> subset-all-triples.csv

# Retrieve all parent relationships
sed -n "/$subpattern/p" nodes.dmp | awk 'BEGIN {FS="|"} {print $1,", Broader Terms ,", $2}' >> subset-all-triples.csv

# Sort it to keep indivitual terms mostly together
sort -g subset-all-triples.csv | uniq > subset-all-triples.uniq; mv subset-all-triples.uniq  subset-all-triples.csv;
