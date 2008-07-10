NLM JOURNAL PUBLISHING DTD

README.TXT                                 3 January, 2005

This README names the modules delivered as Version 2.0 
of the NLM Journal Publishing DTD.

A separate file will contain a description of the major
changes made between Version 1.1 and Version 2.0. These
changes are fully backwards compatible for XML documents
(document instances) That is, all documents that were valid 
to Version 1.0 or 1.1 will be valid to Version 2.0. The 
DTDs are not fully backwards compatible for all  
DTD-customizations. That is, customizations made to 
Versions 1.0 and 1.1 will simply not "work" with Version 2.0.
This is largely because the modularization for customizing
has been revised to make small-scale customization easier,
and the customization technique is easier and more powerful
but different. As another example, some Parameter Entity 
names have been changed.


The rest of this README contains notes concerning:
  1.0 Catalog File
  2.0 Modules Specific to the NLM Journal Publishing DTD
  3.0 Critical Base Modules
  4.0 Element Class Modules
  5.0 Math Modules 
  6.0 Table Modules
  7.0 Notations and Special Characters
  8.0 Sample files for testing


------------------------------------------------------
1.0 Catalog File

catalog.ent - OASIS SOCAT catalog of the fpi and file names
              Not part of the DTD Suite proper, but may be
              used to implement a system using the suite.


------------------------------------------------------
2.0 Modules Specific to the NLM Journal Publishing DTD

In versions 1.0 and 1.1 of the NLM Journal Publishing DTD,
all class definitions, mix definitions, and DTD-over-rides
were done in a single module, named journalpubcustomize.ent.
This module has been replaced by several smaller, single- 
purpose modules, to make it obvious both what has been
over-ridden and what can be over-ridden using the recommended
customization methods and modules.

journalpublishing.dtd          
             - Main DTD for NLM Journal Publishing DTD,
               this is the DOCTYPE that covers a journal
               article and various other non-article
               journal content such as book and product 
               reviews. This DTD invokes almost all the 
               modules in the NLM Archiving and Interchange
               DTD Suite.

journalpubcustom-modules.ent 
             - Names all new modules created specifically
               for this DTD (therefore not part of the
               base DTD Suite)
               (Must be called as the second module in 
               the DTD, just before the Suite Module
               of Modules %modules.ent;)

journalpubcustom-classes.ent
             - the DTD-specific class definitions for the 
               this DTD. Used to over-ride the Suite
               default classes. 
               Declared in %journalpubcustom-modules.ent;
               Must be invoked before the default classes
               module.

journalpubcustom-mixes.ent
             - the DTD-specific mix definitions for the 
               this DTD. Used to over-ride the Suite
               default mixes. 
               Declared in %journalpubcustom-modules.ent;
               Must be invoked before the default mixes
               module.

journalpubcustom-models.ent
             - the DTD-specific content model definitions 
               for the this DTD. Used to over-ride the Suite
               default models. 
               Declared in %journalpubcustom-modules.ent;
               Must be invoked before the all the DTD Suite
               modules since it is used to over0ride them.
            
               There are two types of such over-rides. Those 
               that replace a complete content model are
               named with a suffix "-model". Those that are 
               OR-groups of elements, intended to be mixed 
               with #PCDATA inside a particular model are 
               named with an "-elements" suffix.

------------------------------------------------------
3.0 Critical Base Modules

modules.ent      - Names all the modules in the NLM 
                   Archiving and Interchange DTD Suite
                   (Must be called as the second module
                   by any DTD, after the DTD-specific module
                   of modules and before all other modules). 
                   NOTE: May name modules (such as the 
                   OASIS-Exchange module) that are not called 
                   by a particular DTD.

common.ent       - Defines all elements, attributes, entities
                   used by more than one module
                   (Called after all customization and module-of-
                   module modules but before all the class
                   modules.)

These modules need to be invoked before all other modules 
in a DTD. Other modules can usually be invoked in any order.


------------------------------------------------------
4.0 Element Class Modules (define elements, attributes for one class)

articlemeta.ent  - Article-level metadata elements 
backmatter.ent   - Article-level back matter elements
display.ent      - Display elements such as Table, Figure, Graphic
format.ent       - Format-related elements such as Bold
journalmeta.ent  - Journal-level metadata elements
link.ent         - Linking elements such as X(Cross)-Reference
list.ent         - List elements
math.ent         - NLM-defined math elements such as Display Equation
para.ent         - Paragraph-level elements such as Paragraph and
                      Block Quote
phrase.ent       - Phrase-level content-related elements
references.ent   - Bibliographic reference list and the elements
                      that can be used inside a citation
section.ent      - Section-level elements


------------------------------------------------------
5.0 Math Modules (Define MathML tagging, used in math.ent)

These modules are defined in the suite and should be invoked
from the DTD if MathML tagging is desired.

mathml2.dtd
mathmlsetup.ent

And inside the mathml subdirectory:
  mathml2-qname-1.mod
  mmlalias.ent
  mmlextra.ent

 
------------------------------------------------------
6.0 Table Modules (Define XHTML and OASIS Exchange Table Models))

These modules are defined in the suite and should be invoked
from the DTD if table tagging is desired.

Using either table model requires two modules: one to set up the
Parameter Entities necessary to use the model, and the second to
define the model itself (as defined publicly).

XHTML Table Model
  XHTMLtablesetup.ent
  htmltable.dtd

NLM Version (with namespaces) of OASIS Exchange Table Model
  oasis-tablesetup.ent
  oasis-exchange.ent


------------------------------------------------------
7.0 Notations and Special Characters

notat.ent        - Names all Notations used
xmlspecchars.ent - Names all the standard special character
                      entity sets to be used by the DTD. The
                      MathML characters sets were used,
                      unchanged
chars.ent        - Definitions of NLM-specific and custom
                      special characters (as general entities
                      defined as hexadecimal or decimal character
                      entities - Unicode numbers or using the
                      <private-char> element.) 

All the MathML special character entity sets
(inside the xmlchars directory)
  isoamsa.ent
  isoamsb.ent
  isoamsc.ent
  isoamsn.ent
  isoamso.ent
  isoamsr.ent
  isobox.ent
  isocyr1.ent
  isocyr2.ent
  isodia.ent
  isogrk1.ent
  isogrk2.ent
  isogrk3.ent
  isogrk4.ent
  isolat1.ent
  isolat2.ent
  isomfrk.ent
  isomopf.ent
  isomscr.ent
  isonum.ent
  isopub.ent
  isotech.ent
 
 
------------------------------------------------------
8.0 Sample files for testing

samplesmall-pub.xml - Minimal journal article document 
                      used to test the DTD

------------ document end ----------------------------



