<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="./common.xsl"/>

<!--
<xsl:output method="xml" encoding="UTF-8"
    indent="no" doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
    doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"/>
-->

<xsl:template match="/">
  <ul>
    <xsl:apply-templates select="article/body/sec"/>
    <xsl:apply-templates select="article/back/*"/>
  </ul>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="sec">
  <li>
    <span class="label">
      <xsl:apply-templates select="." mode="num"/>
    </span>
    <a>
      <xsl:attribute name="href">
        <xsl:text>#</xsl:text>
        <xsl:apply-templates select="." mode="id"/>
      </xsl:attribute>
      <xsl:apply-templates select="title"/>
    </a>
    <xsl:if test="sec">
      <ul>
        <xsl:apply-templates select="sec"/>
      </ul>
    </xsl:if>
  </li>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="title">
  <xsl:apply-templates/>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="ack|ref-list|glossary">
  <li>
    <a>
      <xsl:attribute name="href">
        <xsl:text>#</xsl:text>
        <xsl:apply-templates select="." mode="id"/>
      </xsl:attribute>
      <xsl:if test="not(title)">
        <xsl:apply-templates select="." mode="gentitle"/>
      </xsl:if>
      <xsl:apply-templates select="title"/>
    </a>
  </li>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="*">
  <!-- suppress unwanted stuff -->
</xsl:template>


<!-- **************************************************************** -->
<xsl:template name="maketitle">
  <xsl:param name="title" select="''"/>
  <xsl:param name="depth" select="0"/>
  <xsl:copy-of select="$title"/>
</xsl:template>


<!-- **************************************************************** -->
<!-- * THIS IS A SAMPLE OUTPUT ************************************** -->
<!-- **************************************************************** -->
<xsl:template match="/" mode="SAMPLE">
  <ul>
    <li><span class="label">1</span> <a href="#s1">Introduction</a></li>
    <li><span class="label">2</span> <a href="#s2">Results</a>
      <ul>
        <li><span class="label">2.1</span> <a href="#s2a">NCA Is Required for Synaptic Transmission at GABAergic and Cholinergic Neuromuscular Junctions (NMJs)</a></li>
        <li><span class="label">2.2</span> <a href="#s2b">NCA Activity Regulates Presynaptic Activation at Serotonergic NMJs</a></li>
        <li><span class="label">2.3</span> <a href="#s2c">NCNCA Activity Depends on UNC-79 and UNC-80, a Large, Novel Protein</a></li>
        <li><span class="label">2.4</span> <a href="#s2d">NCA-1 and UNC-80 Are Expressed and Function in Neuronal Processes</a></li>
        <li><span class="label">2.5</span> <a href="#s2e">UNC-79, UNC-80, and NCA-1 Facilitate Each Other's Localization</a></li>
      </ul>
    </li>
    <li><span class="label">3</span> <a href="#s3">Discussion</a>
      <ul>
        <li><span class="label">3.1</span> <a href="#s3a">A Putative NCA Channel Transmits Depolarization Signals in <i>C. elegans</i> Neurons</a></li>
        <li><span class="label">3.2</span> <a href="#s3b">The Effects of Gain-of-Function Mutations on Neuronal Excitability</a></li>
        <li><span class="label">3.3</span> <a href="#s3c">Both UNC-80 and UNC-79 Regulate the Putative NCA Channel through Localizing the Pore-Forming Subunit</a></li>
      </ul>
    </li>
    <li><span class="label">4</span> <a href="#s4">Materials and Methods</a>
      <ul>
        <li><span class="label">4.1</span> <a href="#s4a">Strains.</a></li>
        <li><span class="label">4.2</span> <a href="#s4b">Identification, mapping, and cloning of hp102 and unc-77(e625).</a></li>
        <li><span class="label">4.3</span> <a href="#s4c">Identification, mapping, and cloning of unc-80.</a></li>
      </ul>
    </li>
    <li><span class="label">5</span> <a href="#s5">Supporting Information</a></li>
  </ul>
</xsl:template>

</xsl:stylesheet>
