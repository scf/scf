<?xml version="1.0" encoding="utf-8"?>
<!-- ============================================================= -->
<!--  BIBLIO RULES EXTRACTED FROM ViewNLM-v2.3.xsl                 -->
<!--  see: http://dtd.nlm.nih.gov/tools/                           -->
<!--                                                               -->
<!--  MODULE:    HTML View of NLM Journal Article                  -->
<!--  VERSION:   2.30                                              -->
<!--  DATE:      June 2007                                         -->
<!-- ============================================================= -->
<xsl:stylesheet
  version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xlink="http://www.w3.org/1999/xlink"
  exclude-result-prefixes="xsl xlink">

<!-- **************************************************************** -->
<!--
<xsl:template match="*" mode="none">
  <xsl:apply-templates mode="none"/>
</xsl:template>
-->


<!-- **************************************************************** -->
<xsl:template match="*" mode="nscitation">
  <xsl:apply-templates mode="none"/>
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="ref//person-group/name">
  <xsl:value-of select="surname"/>
  <xsl:text>, </xsl:text>
  <xsl:value-of select="given-names"/>
  <xsl:text>. </xsl:text>
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="ref//etal">
  <xsl:text>et al. </xsl:text>
</xsl:template>


	<!-- ============================================================= -->
	<!--  54. CITATION (for NLM Archiving DTD)                         -->
	<!-- ============================================================= -->

	<!-- The citation model is mixed-context, so it is processed
     with an apply-templates (as for a paragraph)
       -except-
     if there is no PCDATA (only elements), spacing and punctuation
     also must be supplied = mode nscitation. -->

	<xsl:template match="ref/citation">

		<xsl:choose>
			<!-- if has no significant text content, presume that
           punctuation is not supplied in the source XML
           = transform will supply it. -->
			<xsl:when test="not(text()[normalize-space()])">
				<xsl:apply-templates select="*" mode="none"/>
			</xsl:when>

			<!-- mixed-content, processed as paragraph -->
			<xsl:otherwise>
				<xsl:apply-templates mode="nscitation"/>
			</xsl:otherwise>
		</xsl:choose>

	</xsl:template>


	<!-- ============================================================= -->
	<!--  55. NLM-CITATION (for NLM Publishing DTD)                    -->
	<!-- ============================================================= -->

	<!-- The nlm-citation model allows only element content, so
     it takes a pull template and adds punctuation. -->

	<!-- Processing of nlm-citation uses several modes, including
     citation, book, edited-book, conf, inconf, and mode "none".   -->

	<!-- Each citation-type is handled in its own template. -->


	<!-- Book or thesis -->
	<xsl:template
		match="ref/nlm-citation[@citation-type='book']
                   | ref/nlm-citation[@citation-type='thesis']">

		<xsl:variable name="augroupcount" select="count(person-group) + count(collab)"/>

		<xsl:choose>

			<xsl:when
				test="$augroupcount>1 and
                    person-group[@person-group-type!='author'] and
                    article-title ">
				<xsl:apply-templates select="person-group[@person-group-type='author']" mode="book"/>
				<xsl:apply-templates select="collab" mode="book"/>
				<xsl:apply-templates select="article-title" mode="editedbook"/>
				<xsl:text>In: </xsl:text>
				<xsl:apply-templates
					select="person-group[@person-group-type='editor']
                                 | person-group[@person-group-type='allauthors']
                                 | person-group[@person-group-type='translator']
                                 | person-group[@person-group-type='transed'] "
					mode="book"/>
				<xsl:apply-templates select="source" mode="book"/>
				<xsl:apply-templates select="edition" mode="book"/>
				<xsl:apply-templates select="volume" mode="book"/>
				<xsl:apply-templates select="trans-source" mode="book"/>
				<xsl:apply-templates select="publisher-name | publisher-loc" mode="none"/>
				<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
				<xsl:apply-templates select="fpage | lpage" mode="book"/>
			</xsl:when>

			<xsl:when
				test="person-group[@person-group-type='author'] or
                    person-group[@person-group-type='compiler']">
				<xsl:apply-templates
					select="person-group[@person-group-type='author']
                                 | person-group[@person-group-type='compiler']"
					mode="book"/>
				<xsl:apply-templates select="collab" mode="book"/>
				<xsl:apply-templates select="source" mode="book"/>
				<xsl:apply-templates select="edition" mode="book"/>
				<xsl:apply-templates
					select="person-group[@person-group-type='editor']
                                 | person-group[@person-group-type='translator']
                                 | person-group[@person-group-type='transed'] "
					mode="book"/>
				<xsl:apply-templates select="volume" mode="book"/>
				<xsl:apply-templates select="trans-source" mode="book"/>
				<xsl:apply-templates select="publisher-name | publisher-loc" mode="none"/>
				<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
				<xsl:apply-templates select="article-title | fpage | lpage" mode="book"/>
			</xsl:when>

			<xsl:otherwise>
				<xsl:apply-templates
					select="person-group[@person-group-type='editor']
                                 | person-group[@person-group-type='translator']
                                 | person-group[@person-group-type='transed']
                                 | person-group[@person-group-type='guest-editor']"
					mode="book"/>
				<xsl:apply-templates select="collab" mode="book"/>
				<xsl:apply-templates select="source" mode="book"/>
				<xsl:apply-templates select="edition" mode="book"/>
				<xsl:apply-templates select="volume" mode="book"/>
				<xsl:apply-templates select="trans-source" mode="book"/>
				<xsl:apply-templates select="publisher-name | publisher-loc" mode="none"/>
				<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
				<xsl:apply-templates select="article-title | fpage | lpage" mode="book"/>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:call-template name="citation-tag-ends"/>
	</xsl:template>


	<!-- Conference proceedings -->
	<xsl:template match="ref/nlm-citation[@citation-type='confproc']">

		<xsl:variable name="augroupcount" select="count(person-group) + count(collab)"/>

		<xsl:choose>
			<xsl:when test="$augroupcount>1 and person-group[@person-group-type!='author']">
				<xsl:apply-templates select="person-group[@person-group-type='author']" mode="book"/>
				<xsl:apply-templates select="collab"/>
				<xsl:apply-templates select="article-title" mode="inconf"/>
				<xsl:text>In: </xsl:text>
				<xsl:apply-templates
					select="person-group[@person-group-type='editor']
                                 | person-group[@person-group-type='allauthors']
                                 | person-group[@person-group-type='translator']
                                 | person-group[@person-group-type='transed'] "
					mode="book"/>
				<xsl:apply-templates select="source" mode="conf"/>
				<xsl:apply-templates select="conf-name | conf-date | conf-loc" mode="conf"/>
				<xsl:apply-templates select="publisher-loc" mode="none"/>
				<xsl:apply-templates select="publisher-name" mode="none"/>
				<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
				<xsl:apply-templates select="fpage | lpage" mode="book"/>
			</xsl:when>

			<xsl:otherwise>
				<xsl:apply-templates select="person-group" mode="book"/>
				<xsl:apply-templates select="collab" mode="book"/>
				<xsl:apply-templates select="article-title" mode="conf"/>
				<xsl:apply-templates select="source" mode="conf"/>
				<xsl:apply-templates select="conf-name | conf-date | conf-loc" mode="conf"/>
				<xsl:apply-templates select="publisher-loc" mode="none"/>
				<xsl:apply-templates select="publisher-name" mode="none"/>
				<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
				<xsl:apply-templates select="fpage | lpage" mode="book"/>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:call-template name="citation-tag-ends"/>
	</xsl:template>


	<!-- Government and other reports, other, web, and commun -->
	<xsl:template
		match="ref/nlm-citation[@citation-type='gov']
                   | ref/nlm-citation[@citation-type='web']
                   | ref/nlm-citation[@citation-type='commun']
                   | ref/nlm-citation[@citation-type='other']">

		<xsl:apply-templates select="person-group" mode="book"/>

		<xsl:apply-templates select="collab"/>

		<xsl:choose>
			<xsl:when test="publisher-loc | publisher-name">
				<xsl:apply-templates select="source" mode="book"/>
				<xsl:choose>
					<xsl:when test="@citation-type='web'">
						<xsl:apply-templates select="edition" mode="none"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:apply-templates select="edition"/>
					</xsl:otherwise>
				</xsl:choose>

				<xsl:apply-templates select="publisher-loc" mode="none"/>
				<xsl:apply-templates select="publisher-name" mode="none"/>
				<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
				<xsl:apply-templates select="article-title|gov" mode="none"/>
			</xsl:when>

			<xsl:otherwise>
				<xsl:apply-templates select="article-title|gov" mode="book"/>
				<xsl:apply-templates select="source" mode="book"/>
				<xsl:apply-templates select="edition"/>
				<xsl:apply-templates select="publisher-loc" mode="none"/>
				<xsl:apply-templates select="publisher-name" mode="none"/>
				<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:apply-templates select="fpage | lpage" mode="book"/>

		<xsl:call-template name="citation-tag-ends"/>

	</xsl:template>


	<!-- Patents  -->
	<xsl:template match="ref/nlm-citation[@citation-type='patent']">

		<xsl:apply-templates select="person-group" mode="book"/>
		<xsl:apply-templates select="collab" mode="book"/>
		<xsl:apply-templates select="article-title | trans-title" mode="none"/>
		<xsl:apply-templates select="source" mode="none"/>
		<xsl:apply-templates select="patent" mode="none"/>
		<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
		<xsl:apply-templates select="fpage | lpage" mode="book"/>

		<xsl:call-template name="citation-tag-ends"/>

	</xsl:template>


	<!-- Discussion  -->
	<xsl:template match="ref/nlm-citation[@citation-type='discussion']">

		<xsl:apply-templates select="person-group" mode="book"/>
		<xsl:apply-templates select="collab"/>
		<xsl:apply-templates select="article-title" mode="editedbook"/>
		<xsl:text>In: </xsl:text>
		<xsl:apply-templates select="source" mode="none"/>

		<xsl:if test="publisher-name | publisher-loc">
			<xsl:text> [</xsl:text>
			<xsl:apply-templates select="publisher-loc" mode="none"/>
			<xsl:value-of select="publisher-name"/>
			<xsl:text>]; </xsl:text>
		</xsl:if>

		<xsl:apply-templates select="year | month | time-stamp | season | access-date" mode="book"/>
		<xsl:apply-templates select="fpage | lpage" mode="book"/>

		<xsl:call-template name="citation-tag-ends"/>
	</xsl:template>


	<!-- If none of the above citation-types applies,
     use mode="none". This generates punctuation. -->
	<!-- (e.g., citation-type="journal"              -->
	<xsl:template match="nlm-citation">

		<xsl:apply-templates
			select="*[not(self::annotation) and
                                 not(self::edition) and
                                 not(self::lpage) and
                                 not(self::comment)]|text()"
			mode="none"/>

		<xsl:call-template name="citation-tag-ends"/>

	</xsl:template>


	<!-- ============================================================= -->
	<!-- person-group, mode=book                                       -->
	<!-- ============================================================= -->

	<xsl:template match="person-group" mode="book">

		<!-- XX needs fix, value is not a nodeset on the when -->
		<!--
  <xsl:choose>

    <xsl:when test="@person-group-type='editor'
                  | @person-group-type='assignee'
                  | @person-group-type='translator'
                  | @person-group-type='transed'
                  | @person-group-type='guest-editor'
                  | @person-group-type='compiler'
                  | @person-group-type='inventor'
                  | @person-group-type='allauthors'">

      <xsl:call-template name="make-persons-in-mode"/>
      <xsl:call-template name="choose-person-type-string"/>
      <xsl:call-template name="choose-person-group-end-punct"/>

    </xsl:when>

    <xsl:otherwise>
      <xsl:apply-templates mode="book"/>
    </xsl:otherwise>

  </xsl:choose>
-->

		<xsl:call-template name="make-persons-in-mode"/>
		<xsl:call-template name="choose-person-type-string"/>
		<xsl:call-template name="choose-person-group-end-punct"/>

	</xsl:template>



	<!-- if given names aren't all-caps, use book mode -->

	<xsl:template name="make-persons-in-mode">

		<xsl:variable name="gnms" select="string(descendant::given-names)"/>

		<xsl:variable name="GNMS"
			select="translate($gnms,
      'abcdefghjiklmnopqrstuvwxyz',
      'ABCDEFGHJIKLMNOPQRSTUVWXYZ')"/>

		<xsl:choose>
			<xsl:when test="$gnms=$GNMS">
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:apply-templates mode="book"/>
			</xsl:otherwise>
		</xsl:choose>

	</xsl:template>


	<xsl:template name="choose-person-type-string">

		<xsl:variable name="person-group-type">
			<xsl:value-of select="@person-group-type"/>
		</xsl:variable>

		<xsl:choose>
			<!-- allauthors is an exception to the usual choice pattern -->
			<xsl:when test="$person-group-type='allauthors'"/>

			<!-- the usual choice pattern: singular or plural? -->
			<xsl:when test="count(name) > 1 or etal ">
				<xsl:text>, </xsl:text>
				<xsl:value-of select="($person-strings[@source=$person-group-type]/@plural)"/>
			</xsl:when>

			<xsl:otherwise>
				<xsl:text>, </xsl:text>
				<xsl:value-of select="($person-strings[@source=$person-group-type]/@singular)"/>
			</xsl:otherwise>
		</xsl:choose>

	</xsl:template>


	<xsl:template name="choose-person-group-end-punct">

		<xsl:choose>
			<!-- compiler is an exception to the usual choice pattern -->
			<xsl:when test="@person-group-type='compiler'">
				<xsl:text>. </xsl:text>
			</xsl:when>

			<!-- the usual choice pattern: semi-colon or period? -->
			<xsl:when test="following-sibling::person-group">
				<xsl:text>; </xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>

	</xsl:template>


	<!-- ============================================================= -->
	<!--  56. Citation subparts (mode "none" separately at end)        -->
	<!-- ============================================================= -->

	<!-- names -->

	<xsl:template match="name" mode="nscitation">
		<xsl:value-of select="surname"/>
		<xsl:text>, </xsl:text>
		<xsl:value-of select="given-names"/>
		<xsl:text>. </xsl:text>
	</xsl:template>


	<xsl:template match="name" mode="book">
		<xsl:variable name="nodetotal" select="count(../*)"/>
		<xsl:variable name="penult" select="count(../*)-1"/>
		<xsl:variable name="position" select="position()"/>

		<xsl:choose>

			<!-- if given-names -->
			<xsl:when test="given-names">
				<xsl:apply-templates select="surname"/>
				<xsl:text>, </xsl:text>
				<xsl:call-template name="firstnames">
					<xsl:with-param name="nodetotal" select="$nodetotal"/>
					<xsl:with-param name="position" select="$position"/>
					<xsl:with-param name="names" select="given-names"/>
					<xsl:with-param name="pgtype">
						<xsl:choose>
							<xsl:when test="parent::person-group[@person-group-type]">
								<xsl:value-of select="parent::person-group/@person-group-type"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="'author'"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:with-param>
				</xsl:call-template>

				<xsl:if test="suffix">
					<xsl:text>, </xsl:text>
					<xsl:apply-templates select="suffix"/>
				</xsl:if>
			</xsl:when>

			<!-- if no given-names -->
			<xsl:otherwise>
				<xsl:apply-templates select="surname"/>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:choose>
			<!-- if have aff -->
			<xsl:when test="following-sibling::aff"/>

			<!-- if don't have aff -->
			<xsl:otherwise>
				<xsl:choose>

					<!-- if part of person-group -->
					<xsl:when test="parent::person-group/@person-group-type">
						<xsl:choose>

							<!-- if author -->
							<xsl:when test="parent::person-group/@person-group-type='author'">
								<xsl:choose>
									<xsl:when test="$nodetotal=$position">. </xsl:when>
									<xsl:when test="$penult=$position">
										<xsl:choose>
											<xsl:when test="following-sibling::etal">, </xsl:when>
											<xsl:otherwise>; </xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>; </xsl:otherwise>
								</xsl:choose>
							</xsl:when>

							<!-- if not author -->
							<xsl:otherwise>
								<xsl:choose>
									<xsl:when test="$nodetotal=$position"/>
									<xsl:when test="$penult=$position">
										<xsl:choose>
											<xsl:when test="following-sibling::etal">, </xsl:when>
											<xsl:otherwise>; </xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>; </xsl:otherwise>
								</xsl:choose>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>

					<!-- if not part of person-group -->
					<xsl:otherwise>
						<xsl:choose>
							<xsl:when test="$nodetotal=$position">. </xsl:when>
							<xsl:when test="$penult=$position">
								<xsl:choose>
									<xsl:when test="following-sibling::etal">, </xsl:when>
									<xsl:otherwise>; </xsl:otherwise>
								</xsl:choose>
							</xsl:when>
							<xsl:otherwise>; </xsl:otherwise>
						</xsl:choose>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>

		</xsl:choose>
	</xsl:template>


	<xsl:template match="collab" mode="book">
		<xsl:apply-templates/>
		<xsl:if test="@collab-type='compilers'">
			<xsl:text>, </xsl:text>
			<xsl:value-of select="@collab-type"/>
		</xsl:if>
		<xsl:if test="@collab-type='assignee'">
			<xsl:text>, </xsl:text>
			<xsl:value-of select="@collab-type"/>
		</xsl:if>
		<xsl:text>. </xsl:text>
	</xsl:template>

	<xsl:template match="etal" mode="book">
		<xsl:text>et al.</xsl:text>
		<xsl:choose>
			<xsl:when test="parent::person-group/@person-group-type">
				<xsl:choose>
					<xsl:when test="parent::person-group/@person-group-type='author'">
						<xsl:text> </xsl:text>
					</xsl:when>
					<xsl:otherwise/>
				</xsl:choose>
			</xsl:when>

			<xsl:otherwise>
				<xsl:text> </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- affiliations -->

	<xsl:template match="aff" mode="book">
		<xsl:variable name="nodetotal" select="count(../*)"/>
		<xsl:variable name="position" select="position()"/>

		<xsl:text> (</xsl:text>
		<xsl:apply-templates/>
		<xsl:text>)</xsl:text>

		<xsl:choose>
			<xsl:when test="$nodetotal=$position">. </xsl:when>
			<xsl:otherwise>, </xsl:otherwise>
		</xsl:choose>
	</xsl:template>



	<!-- publication info -->

	<xsl:template match="article-title" mode="nscitation">
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="article-title" mode="book">
		<xsl:apply-templates/>

		<xsl:choose>
			<xsl:when test="../fpage or ../lpage">
				<xsl:text>; </xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="article-title" mode="editedbook">
		<xsl:apply-templates/>
		<xsl:text>. </xsl:text>
	</xsl:template>

	<xsl:template match="article-title" mode="conf">
		<xsl:apply-templates/>
		<xsl:choose>
			<xsl:when test="../conf-name">
				<xsl:text>. </xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>; </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="article-title" mode="inconf">
		<xsl:apply-templates/>
		<xsl:text>. </xsl:text>
	</xsl:template>



	<xsl:template match="source" mode="nscitation">
		<i>
			<xsl:apply-templates/>
		</i>
	</xsl:template>

	<xsl:template match="source" mode="book">
		<xsl:choose>

			<xsl:when test="../trans-source">
				<xsl:apply-templates/>
				<xsl:choose>
					<xsl:when test="../volume | ../edition">
						<xsl:text>. </xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text> </xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>

			<xsl:otherwise>
				<xsl:apply-templates/>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="source" mode="conf">
		<xsl:apply-templates/>
		<xsl:text>; </xsl:text>
	</xsl:template>

	<xsl:template match="trans-source" mode="book">
		<xsl:text> [</xsl:text>
		<xsl:apply-templates/>
		<xsl:text>]. </xsl:text>
	</xsl:template>

	<xsl:template match="volume" mode="nscitation">
		<xsl:text> </xsl:text>
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="volume | edition" mode="book">
		<xsl:apply-templates/>
		<xsl:if test="@collab-type='compilers'">
			<xsl:text>, </xsl:text>
			<xsl:value-of select="@collab-type"/>
		</xsl:if>
		<xsl:if test="@collab-type='assignee'">
			<xsl:text>, </xsl:text>
			<xsl:value-of select="@collab-type"/>
		</xsl:if>
		<xsl:text>. </xsl:text>
	</xsl:template>

	<!-- dates -->

	<xsl:template match="month" mode="nscitation">
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="month" mode="book">
		<xsl:variable name="month" select="."/>
		<xsl:choose>
			<xsl:when test="$month='01' or $month='1' or $month='January'">Jan</xsl:when>
			<xsl:when test="$month='02' or $month='2' or $month='February'">Feb</xsl:when>
			<xsl:when test="$month='03' or $month='3' or $month='March'">Mar</xsl:when>
			<xsl:when test="$month='04' or $month='4' or $month='April'">Apr</xsl:when>
			<xsl:when test="$month='05' or $month='5' or $month='May'">May</xsl:when>
			<xsl:when test="$month='06' or $month='6' or $month='June'">Jun</xsl:when>
			<xsl:when test="$month='07' or $month='7' or $month='July'">Jul</xsl:when>
			<xsl:when test="$month='08' or $month='8' or $month='August'">Aug</xsl:when>
			<xsl:when test="$month='09' or $month='9' or $month='September'">Sep</xsl:when>
			<xsl:when test="$month='10' or $month='October'">Oct</xsl:when>
			<xsl:when test="$month='11' or $month='November'">Nov</xsl:when>
			<xsl:when test="$month='12' or $month='December'">Dec</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$month"/>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:if test="../day">
			<xsl:text> </xsl:text>
			<xsl:value-of select="../day"/>
		</xsl:if>

		<xsl:choose>
			<xsl:when test="../time-stamp">
				<xsl:text>, </xsl:text>
				<xsl:value-of select="../time-stamp"/>
				<xsl:text> </xsl:text>
			</xsl:when>
			<xsl:when test="../access-date"/>
			<xsl:otherwise>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<xsl:template match="day" mode="nscitation">
		<xsl:apply-templates/>
	</xsl:template>


	<xsl:template match="year" mode="nscitation">
		<xsl:text> </xsl:text>
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="year" mode="book">
		<xsl:choose>
			<xsl:when test="../month or ../season or ../access-date">
				<xsl:apply-templates/>
				<xsl:text> </xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:apply-templates/>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>



	<xsl:template match="time-stamp" mode="nscitation">
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="time-stamp" mode="book"/>


	<xsl:template match="access-date" mode="nscitation">
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="access-date" mode="book">
		<xsl:text> [</xsl:text>
		<xsl:apply-templates/>
		<xsl:text>]. </xsl:text>
	</xsl:template>



	<xsl:template match="season" mode="book">
		<xsl:apply-templates/>
		<xsl:if test="@collab-type='compilers'">
			<xsl:text>, </xsl:text>
			<xsl:value-of select="@collab-type"/>
		</xsl:if>
		<xsl:if test="@collab-type='assignee'">
			<xsl:text>, </xsl:text>
			<xsl:value-of select="@collab-type"/>
		</xsl:if>
		<xsl:text>. </xsl:text>
	</xsl:template>



	<!-- pages -->

	<xsl:template match="fpage" mode="nscitation">
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="fpage" mode="book">
		<xsl:text>p. </xsl:text>
		<xsl:apply-templates/>

		<xsl:if test="../lpage">
			<xsl:text>.</xsl:text>
		</xsl:if>

	</xsl:template>


	<xsl:template match="lpage" mode="book">
		<xsl:choose>
			<xsl:when test="../fpage">
				<xsl:text>-</xsl:text>
				<xsl:apply-templates/>
				<xsl:text>.</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:apply-templates/>
				<xsl:text> p.</xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="lpage" mode="nscitation">
		<xsl:apply-templates/>
	</xsl:template>

	<!-- misc stuff -->

	<xsl:template match="pub-id" mode="nscitation">
		<xsl:text> [</xsl:text>
		<xsl:value-of select="@pub-id-type"/>
		<xsl:text>: </xsl:text>
		<xsl:apply-templates/>
		<xsl:text>]</xsl:text>
	</xsl:template>

	<xsl:template match="annotation" mode="nscitation">
		<blockquote>
			<xsl:apply-templates/>
		</blockquote>
	</xsl:template>

	<xsl:template match="comment" mode="nscitation">
		<xsl:if test="not(self::node()='.')">
			<br/>
			<small>
				<xsl:apply-templates/>
			</small>
		</xsl:if>
	</xsl:template>

	<xsl:template match="conf-name | conf-date" mode="conf">
		<xsl:apply-templates/>
		<xsl:text>; </xsl:text>
	</xsl:template>

	<xsl:template match="conf-loc" mode="conf">
		<xsl:apply-templates/>
		<xsl:text>. </xsl:text>
	</xsl:template>


	<!-- All formatting elements in citations processed normally -->
	<xsl:template match="bold | italic | monospace | overline | sc | strike | sub |sup | underline" mode="nscitation">
		<xsl:apply-templates select="."/>
	</xsl:template>
	
	<xsl:template match="bold|italic|monospace|overline|sc|strike|sub|sup|underline" mode="none">
		<xsl:apply-templates select="."/>
	</xsl:template>
	

	<!-- ============================================================= -->
	<!--  "firstnames"                                                 -->
	<!-- ============================================================= -->

	<!-- called by match="name" in book mode,
     as part of citation handling
     when given-names is not all-caps -->

	<xsl:template name="firstnames">
		<xsl:param name="nodetotal"/>
		<xsl:param name="position"/>
		<xsl:param name="names"/>
		<xsl:param name="pgtype"/>

		<xsl:variable name="length" select="string-length($names)-1"/>
		<xsl:variable name="gnm" select="substring($names,$length,2)"/>
		<xsl:variable name="GNM">
			<xsl:call-template name="capitalize">
				<xsl:with-param name="str" select="substring($names,$length,2)"/>
			</xsl:call-template>
		</xsl:variable>

		<!--
<xsl:text>Value of $names = [</xsl:text><xsl:value-of select="$names"/><xsl:text>]</xsl:text>
<xsl:text>Value of $length = [</xsl:text><xsl:value-of select="$length"/><xsl:text>]</xsl:text>
<xsl:text>Value of $gnm = [</xsl:text><xsl:value-of select="$gnm"/><xsl:text>]</xsl:text>
<xsl:text>Value of $GNM = [</xsl:text><xsl:value-of select="$GNM"/><xsl:text>]</xsl:text>
-->

		<xsl:if test="$names">
			<xsl:choose>

				<xsl:when test="$gnm=$GNM">
					<xsl:apply-templates select="$names"/>
					<xsl:choose>
						<xsl:when test="$nodetotal!=$position">
							<xsl:text>.</xsl:text>
						</xsl:when>
						<xsl:when test="$pgtype!='author'">
							<xsl:text>.</xsl:text>
						</xsl:when>
					</xsl:choose>
				</xsl:when>

				<xsl:otherwise>
					<xsl:apply-templates select="$names"/>
				</xsl:otherwise>

			</xsl:choose>
		</xsl:if>

	</xsl:template>



	<!-- ============================================================= -->
	<!-- mode=none                                                     -->
	<!-- ============================================================= -->

	<!-- This mode assumes no punctuation is provided in the XML.
     It is used, among other things, for the citation/ref
     when there is no significant text node inside the ref.        -->

	<xsl:template match="name" mode="none">
		<xsl:value-of select="surname"/>
		<xsl:text>, </xsl:text>
		<xsl:value-of select="given-names"/>
		<xsl:text>. </xsl:text>
	</xsl:template>

	<xsl:template match="article-title" mode="none">
		<xsl:apply-templates/>
		<xsl:if test="not(../trans-title)">
			<xsl:text>. </xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template match="volume" mode="none"> 
	  <em>
	  <xsl:apply-templates/>
	  </em>
	</xsl:template>

	<xsl:template match="edition" mode="none">
		<xsl:apply-templates/>
		<xsl:text>. </xsl:text>
	</xsl:template>

	<xsl:template match="supplement" mode="none">
		<xsl:text> </xsl:text>
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="issue" mode="none">
		<xsl:text>(</xsl:text>
		<xsl:apply-templates/>
		<xsl:text>)</xsl:text>
	</xsl:template>

	<xsl:template match="publisher-loc" mode="none">
		<xsl:apply-templates/>
		<xsl:text>: </xsl:text>
	</xsl:template>

	<xsl:template match="publisher-name" mode="none">
		<xsl:apply-templates/>
		<xsl:text>; </xsl:text>
	</xsl:template>

	<xsl:template match="person-group" mode="none">
		<xsl:variable name="gnms" select="string(descendant::given-names)"/>
		<xsl:variable name="GNMS">
			<xsl:call-template name="capitalize">
				<xsl:with-param name="str" select="$gnms"/>
			</xsl:call-template>
		</xsl:variable>

		<xsl:choose>
			<xsl:when test="$gnms=$GNMS">
				<xsl:apply-templates/>
			</xsl:when>

			<xsl:otherwise>
				<xsl:apply-templates select="node()" mode="book"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="collab" mode="none">
		<xsl:apply-templates/>
		<xsl:if test="@collab-type">
			<xsl:text>, </xsl:text>
			<xsl:value-of select="@collab-type"/>
		</xsl:if>

		<xsl:choose>
			<xsl:when test="following-sibling::collab">
				<xsl:text>; </xsl:text>
			</xsl:when>

			<xsl:otherwise>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="source" mode="none">
		<xsl:apply-templates/>

		<xsl:choose>
			<xsl:when test="../access-date">
				<xsl:if test="../edition">
					<xsl:text> (</xsl:text>
					<xsl:apply-templates select="../edition" mode="plain"/>
					<xsl:text>)</xsl:text>
				</xsl:if>
				<xsl:text>. </xsl:text>
			</xsl:when>

			<xsl:when test="../volume | ../fpage">
				<xsl:if test="../edition">
					<xsl:text> (</xsl:text>
					<xsl:apply-templates select="../edition" mode="plain"/>
					<xsl:text>)</xsl:text>
				</xsl:if>
				<xsl:text> </xsl:text>
			</xsl:when>

			<xsl:otherwise>
				<xsl:if test="../edition">
					<xsl:text> (</xsl:text>
					<xsl:apply-templates select="../edition" mode="plain"/>
					<xsl:text>)</xsl:text>
				</xsl:if>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="trans-title" mode="none">
		<xsl:text> [</xsl:text>
		<xsl:apply-templates/>
		<xsl:text>]. </xsl:text>
	</xsl:template>

	<xsl:template match="month" mode="none">
		<xsl:variable name="month" select="."/>
		<xsl:choose>
			<xsl:when test="$month='01' or $month='1' ">Jan</xsl:when>
			<xsl:when test="$month='02' or $month='2' ">Feb</xsl:when>
			<xsl:when test="$month='03' or $month='3' ">Mar</xsl:when>
			<xsl:when test="$month='04' or $month='4' ">Apr</xsl:when>
			<xsl:when test="$month='05' or $month='5' ">May</xsl:when>
			<xsl:when test="$month='06' or $month='6'">Jun</xsl:when>
			<xsl:when test="$month='07' or $month='7'">Jul</xsl:when>
			<xsl:when test="$month='08' or $month='8' ">Aug</xsl:when>
			<xsl:when test="$month='09' or $month='9' ">Sep</xsl:when>
			<xsl:when test="$month='10' ">Oct</xsl:when>
			<xsl:when test="$month='11' ">Nov</xsl:when>
			<xsl:when test="$month='12' ">Dec</xsl:when>

			<xsl:otherwise>
				<xsl:value-of select="$month"/>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:if test="../day">
			<xsl:text> </xsl:text>
			<xsl:value-of select="../day"/>
		</xsl:if>

		<xsl:text>;</xsl:text>

	</xsl:template>

	<xsl:template match="day" mode="none"/>

	<xsl:template match="year" mode="none">
		<xsl:choose>
			<xsl:when test="../month or ../season or ../access-date">
			  <xsl:text> (</xsl:text>
				<xsl:apply-templates mode="none"/>
				<xsl:text>)</xsl:text>
				<xsl:text>. </xsl:text>
			</xsl:when>

			<xsl:otherwise>
			  <xsl:text> (</xsl:text>
				<xsl:apply-templates mode="none"/>
				<xsl:text>). </xsl:text>
				
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="access-date" mode="none">
		<xsl:text> [</xsl:text>
		<xsl:apply-templates/>
		<xsl:text>];</xsl:text>
	</xsl:template>

	<xsl:template match="season" mode="none">
		<xsl:apply-templates/>
		<xsl:text>;</xsl:text>
	</xsl:template>

	<xsl:template match="fpage" mode="none">
		<xsl:variable name="fpgct" select="count(../fpage)"/>
		<xsl:variable name="lpgct" select="count(../lpage)"/>
		<xsl:variable name="hermano" select="name(following-sibling::node())"/>		
		<xsl:choose>
			<xsl:when test="preceding-sibling::fpage">
				<xsl:choose>
					<xsl:when test="following-sibling::fpage">
						<xsl:text> </xsl:text>
						<xsl:apply-templates/>

						<xsl:if test="$hermano='lpage'">
							<xsl:text>&#8211;</xsl:text>
							<xsl:apply-templates select="following-sibling::lpage[1]" mode="none"/>
						</xsl:if>
						<xsl:text>,</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text> </xsl:text>
						<xsl:apply-templates/>
						<xsl:if test="$hermano='lpage'">
							<xsl:text>&#8211;</xsl:text>
							<xsl:apply-templates select="following-sibling::lpage[1]" mode="none"/>
						</xsl:if>
						<xsl:text>.</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>, </xsl:text>
				<xsl:apply-templates/>
				<xsl:choose>
					<xsl:when test="$hermano='lpage'">
						<xsl:text>&#8211;</xsl:text>
						<xsl:apply-templates select="following-sibling::lpage[1]" mode="write"/>
						<xsl:text>.</xsl:text>
					</xsl:when>
					<xsl:when test="$hermano='fpage'">
						<xsl:text>,</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>.</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="lpage" mode="none"/>
	
	<xsl:template match="lpage" mode="write">
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="gov" mode="none">
		<xsl:choose>
			<xsl:when test="../trans-title">
				<xsl:apply-templates/>
			</xsl:when>

			<xsl:otherwise>
				<xsl:apply-templates/>
				<xsl:text>. </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="patent" mode="none">
		<xsl:apply-templates/>
		<xsl:text>. </xsl:text>
	</xsl:template>
	
	<xsl:template match="pub-id" mode="none">
		<xsl:text> [</xsl:text>
		<xsl:value-of select="@pub-id-type"/>
		<xsl:text>: </xsl:text>
		<xsl:apply-templates/>
		<xsl:text>]</xsl:text>
	</xsl:template>
	
	<xsl:template match="comment" mode="none">
		<xsl:text> </xsl:text>
		<xsl:apply-templates/>
	</xsl:template>


	<!-- ============================================================= -->
	<!--  57. "CITATION-TAG-ENDS"                                      -->
	<!-- ============================================================= -->


	<xsl:template name="citation-tag-ends">

		<xsl:apply-templates select="series" mode="citation"/>

		<!-- If language is not English -->
		<!-- XX review logic -->
		<xsl:if test="article-title[@xml:lang!='en']
               or article-title[@xml:lang!='EN']">

			<xsl:call-template name="language">
				<xsl:with-param name="lang" select="article-title/@xml:lang"/>
			</xsl:call-template>
		</xsl:if>

		<xsl:if test="source[@xml:lang!='en']
              or source[@xml:lang!='EN']">

			<xsl:call-template name="language">
				<xsl:with-param name="lang" select="source/@xml:lang"/>
			</xsl:call-template>
		</xsl:if>

		<xsl:apply-templates select="comment" mode="citation"/>

		<xsl:apply-templates select="annotation" mode="citation"/>

	</xsl:template>

</xsl:stylesheet>
