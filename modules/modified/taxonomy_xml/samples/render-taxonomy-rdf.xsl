<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns="http://www.w3.org/1999/xhtml"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" 
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" 
	xmlns:owl="http://www.w3.org/2002/07/owl#"
>
	<xsl:output method = "html" encoding="Windows-1252" />
	
	<xsl:template match="/">	
		<html>
		<head>
			<title></title>
			<style>
			.sub-terms {margin-left:1em;}
			.term .title {font-weight:bold;}
			</style>
		</head>
			<body>
			<xsl:choose>
				<xsl:when test="//owl:Ontology">
					<xsl:apply-templates select="//owl:Ontology"/>
				</xsl:when>
				<xsl:otherwise>
					<em>No vocab defined, trying to list all terms</em>
					<xsl:call-template name="orphaned-terms" />
				</xsl:otherwise>
			</xsl:choose>
			</body>
		</html>
	</xsl:template>
	
	<xsl:template match="owl:Ontology">	
		<div class="vocabulary">
			<h1><xsl:value-of select="rdfs:label" /></h1>
			<h4><xsl:value-of select="owl:versionInfo" /></h4>

			<xsl:call-template name="topterms-of">
				<xsl:with-param name="vocabid" select="concat('#', @rdf:ID)"/>
			</xsl:call-template>
		</div>
	</xsl:template>

	<xsl:template name="topterms-of">
 		<!-- find terms who claim to be in this vocab, and who have no parent -->
		<xsl:param name="vocabid" />
		<xsl:if test="//rdfs:Class[rdfs:isDefinedBy/@rdf:resource = $vocabid]">
			<div class="terms">
				<xsl:apply-templates select="//rdfs:Class[rdfs:isDefinedBy/@rdf:resource = $vocabid and not(rdfs:subClassOf)]"/>
			</div>
		</xsl:if>
	</xsl:template>

	
	<xsl:template match="rdfs:Class">	
		<div class="term">
			<div class="title"><a>
				<xsl:attribute name="href" ><xsl:value-of select="concat('#',@rdf:ID)" /></xsl:attribute>
				<xsl:value-of select="rdfs:label" /> 
			</a></div>
			<div class="description"><xsl:value-of select="rdfs:comment" /></div>

			<xsl:call-template name="listvalues">
				<xsl:with-param name="varname" select="'synonyms'"/>
				<xsl:with-param name="vals" select="owl:equivalentClass"/>
			</xsl:call-template>

			<!-- <div class="parent"><xsl:value-of select="rdfs:subClassOf" /></div> -->

			<xsl:call-template name="subterms-of">
				<xsl:with-param name="parentid" select="concat('#', @rdf:ID)"/>
			</xsl:call-template>

		</div>
	</xsl:template>
	
	<xsl:template name="subterms-of">
		<!-- All terms who name this term as a superclass -->
		<xsl:param name="parentid" />
		<xsl:if test="//rdfs:Class[rdfs:subClassOf/@rdf:resource = $parentid ]">
			<div class="sub-terms">
				<xsl:apply-templates select="//rdfs:Class[rdfs:subClassOf/@rdf:resource = $parentid ]" />
			</div>
		</xsl:if>
	</xsl:template>

	<xsl:template name="listvalues">
	<!-- format a label and possibly multiple values into a small list -->
		<xsl:param name="varname" />
		<xsl:param name="vals" />
		<xsl:if test="$vals" >
			<div class="varname"><xsl:value-of select="$varname" /> : 
				<xsl:for-each select="$vals">
					<span class="value"><xsl:value-of select="." /></span>
				</xsl:for-each>
			</div> 
		</xsl:if>
	</xsl:template>
	
	<xsl:template name="orphaned-terms">
	<!-- select all terms that do not have a VALID parent in this file -->
	<xsl:for-each select="//rdfs:Class">
		<xsl:variable name="parentid"><xsl:value-of select="substring(rdfs:subClassOf/@rdf:resource, 2)" /></xsl:variable>
			<xsl:choose>
				<xsl:when test="rdfs:subClassOf and (//rdfs:Class/@rdf:ID = $parentid)">
				<!-- parent exists, skip for now -->
				</xsl:when>
				<xsl:otherwise>
				<!-- can't find a parent, must be an orphan. Render as normal, it'll do its children -->
				<xsl:apply-templates select="."/>
				</xsl:otherwise>
			</xsl:choose>
	</xsl:for-each>
	</xsl:template>

</xsl:stylesheet>