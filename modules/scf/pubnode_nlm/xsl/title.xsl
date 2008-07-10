<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="nlm23html.xsl"/>

<xsl:output method="xml"/>

<xsl:template match="/">
  <!-- surround with a div because result must always be well-formed doc -->
  <div>
    <xsl:apply-templates select="//title-group/article-title/node()"/>
  </div>
</xsl:template>

</xsl:stylesheet>
