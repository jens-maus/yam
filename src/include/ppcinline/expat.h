/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_EXPAT_H
#define _PPCINLINE_EXPAT_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef EXPAT_BASE_NAME
#define EXPAT_BASE_NAME ExpatBase
#endif /* !EXPAT_BASE_NAME */

#define XML_ParserReset(__p0, __p1) \
  LP2(354, XML_Bool , XML_ParserReset, \
    XML_Parser , __p0, a0, \
    const XML_Char *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetCharacterDataHandler(__p0, __p1) \
  LP2NR(96, XML_SetCharacterDataHandler, \
    XML_Parser , __p0, a0, \
    XML_CharacterDataHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetEndDoctypeDeclHandler(__p0, __p1) \
  LP2NR(192, XML_SetEndDoctypeDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_EndDoctypeDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_StopParser(__p0, __p1) \
  LP2(378, int , XML_StopParser, \
    XML_Parser , __p0, a0, \
    XML_Bool , __p1, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetEncoding(__p0, __p1) \
  LP2(324, int , XML_SetEncoding, \
    XML_Parser , __p0, a0, \
    const XML_Char *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetElementHandler(__p0, __p1, __p2) \
  LP3NR(90, XML_SetElementHandler, \
    XML_Parser , __p0, a0, \
    XML_StartElementHandler , __p1, a1, \
    XML_EndElementHandler , __p2, a2, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetCurrentLineNumber(__p0) \
  LP1(258, int , XML_GetCurrentLineNumber, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetInputContext(__p0, __p1, __p2) \
  LP3(276, const char *, XML_GetInputContext, \
    XML_Parser , __p0, a0, \
    int *, __p1, a1, \
    int *, __p2, a2, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ParserCreate(__p0) \
  (((XML_Parser (*)(const XML_Char *))*(void**)((long)(EXPAT_BASE_NAME) - 514))(__p0))

#define XML_ParseBuffer(__p0, __p1, __p2) \
  LP3(66, int , XML_ParseBuffer, \
    XML_Parser , __p0, a0, \
    int , __p1, d0, \
    int , __p2, d1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ParserCreateNS(__p0, __p1) \
  (((XML_Parser (*)(const XML_Char *, XML_Char ))*(void**)((long)(EXPAT_BASE_NAME) - 520))(__p0, __p1))

#define XML_GetParsingStatus(__p0, __p1) \
  LP2NR(390, XML_GetParsingStatus, \
    XML_Parser , __p0, a0, \
    XML_ParsingStatus *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetBuffer(__p0, __p1) \
  LP2(72, void *, XML_GetBuffer, \
    XML_Parser , __p0, a0, \
    int , __p1, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_Parse(__p0, __p1, __p2, __p3) \
  LP4(60, int , XML_Parse, \
    XML_Parser , __p0, a0, \
    const char *, __p1, a1, \
    int , __p2, d0, \
    int , __p3, d1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetCommentHandler(__p0, __p1) \
  LP2NR(108, XML_SetCommentHandler, \
    XML_Parser , __p0, a0, \
    XML_CommentHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetStartNamespaceDeclHandler(__p0, __p1) \
  LP2NR(162, XML_SetStartNamespaceDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_StartNamespaceDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ResumeParser(__p0) \
  LP1(384, int , XML_ResumeParser, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ErrorString(__p0) \
  LP1(246, const XML_LChar *, XML_ErrorString, \
    int , __p0, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ExternalEntityParserCreate(__p0, __p1, __p2) \
  LP3(48, XML_Parser , XML_ExternalEntityParserCreate, \
    XML_Parser , __p0, a0, \
    const XML_Char *, __p1, a1, \
    const XML_Char *, __p2, a2, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetExternalEntityRefHandlerArg(__p0, __p1) \
  LP2NR(150, XML_SetExternalEntityRefHandlerArg, \
    XML_Parser , __p0, a0, \
    void *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetStartElementHandler(__p0, __p1) \
  LP2NR(78, XML_SetStartElementHandler, \
    XML_Parser , __p0, a0, \
    XML_StartElementHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetErrorCode(__p0) \
  LP1(240, int , XML_GetErrorCode, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_UseParserAsHandlerArg(__p0) \
  LP1NR(294, XML_UseParserAsHandlerArg, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetEndNamespaceDeclHandler(__p0, __p1) \
  LP2NR(168, XML_SetEndNamespaceDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_EndNamespaceDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetAttlistDeclHandler(__p0, __p1) \
  LP2NR(210, XML_SetAttlistDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_AttlistDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetExternalEntityRefHandler(__p0, __p1) \
  LP2NR(144, XML_SetExternalEntityRefHandler, \
    XML_Parser , __p0, a0, \
    XML_ExternalEntityRefHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetEndElementHandler(__p0, __p1) \
  LP2NR(84, XML_SetEndElementHandler, \
    XML_Parser , __p0, a0, \
    XML_EndElementHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetElementDeclHandler(__p0, __p1) \
  LP2NR(204, XML_SetElementDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_ElementDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetNotationDeclHandler(__p0, __p1) \
  LP2NR(228, XML_SetNotationDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_NotationDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ParserCreate_MM(__p0, __p1, __p2) \
  (((XML_Parser (*)(const XML_Char *, const XML_Memory_Handling_Suite *, const XML_Char *))*(void**)((long)(EXPAT_BASE_NAME) - 526))(__p0, __p1, __p2))

#define XML_GetCurrentByteIndex(__p0) \
  LP1(252, long , XML_GetCurrentByteIndex, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetReturnNSTriplet(__p0, __p1) \
  LP2NR(336, XML_SetReturnNSTriplet, \
    XML_Parser , __p0, a0, \
    int , __p1, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetEndCdataSectionHandler(__p0, __p1) \
  LP2NR(120, XML_SetEndCdataSectionHandler, \
    XML_Parser , __p0, a0, \
    XML_EndCdataSectionHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_UseForeignDTD(__p0, __p1) \
  LP2(366, int , XML_UseForeignDTD, \
    XML_Parser , __p0, a0, \
    XML_Bool , __p1, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetXmlDeclHandler(__p0, __p1) \
  LP2NR(180, XML_SetXmlDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_XmlDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ExpatVersionInfo() \
  LP0(348, XML_Expat_Version , XML_ExpatVersionInfo, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetSkippedEntityHandler(__p0, __p1) \
  LP2NR(360, XML_SetSkippedEntityHandler, \
    XML_Parser , __p0, a0, \
    XML_SkippedEntityHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_MemMalloc(__p0, __p1) \
  LP2(402, void *, XML_MemMalloc, \
    XML_Parser , __p0, a0, \
    size_t , __p1, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetIdAttributeIndex(__p0) \
  LP1(318, int , XML_GetIdAttributeIndex, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetCurrentColumnNumber(__p0) \
  LP1(264, int , XML_GetCurrentColumnNumber, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetDoctypeDeclHandler(__p0, __p1, __p2) \
  LP3NR(198, XML_SetDoctypeDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_StartDoctypeDeclHandler , __p1, a1, \
    XML_EndDoctypeDeclHandler , __p2, a2, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetBase(__p0) \
  LP1(306, const XML_Char *, XML_GetBase, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetCdataSectionHandler(__p0, __p1, __p2) \
  LP3NR(126, XML_SetCdataSectionHandler, \
    XML_Parser , __p0, a0, \
    XML_StartCdataSectionHandler , __p1, a1, \
    XML_EndCdataSectionHandler , __p2, a2, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetEntityDeclHandler(__p0, __p1) \
  LP2NR(216, XML_SetEntityDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_EntityDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetCurrentByteCount(__p0) \
  LP1(270, int , XML_GetCurrentByteCount, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetUnknownEncodingHandler(__p0, __p1, __p2) \
  LP3NR(156, XML_SetUnknownEncodingHandler, \
    XML_Parser , __p0, a0, \
    XML_UnknownEncodingHandler , __p1, a1, \
    void *, __p2, a2, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetUnparsedEntityDeclHandler(__p0, __p1) \
  LP2NR(222, XML_SetUnparsedEntityDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_UnparsedEntityDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_MemRealloc(__p0, __p1, __p2) \
  LP3(408, void *, XML_MemRealloc, \
    XML_Parser , __p0, a0, \
    void *, __p1, a1, \
    size_t , __p2, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_MemFree(__p0, __p1) \
  LP2NR(414, XML_MemFree, \
    XML_Parser , __p0, a0, \
    void *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetNotStandaloneHandler(__p0, __p1) \
  LP2NR(234, XML_SetNotStandaloneHandler, \
    XML_Parser , __p0, a0, \
    XML_NotStandaloneHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetUserData(__p0, __p1) \
  LP2NR(282, XML_SetUserData, \
    XML_Parser , __p0, a0, \
    void *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetBase(__p0, __p1) \
  LP2(300, int , XML_SetBase, \
    XML_Parser , __p0, a0, \
    const XML_Char *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_DefaultCurrent(__p0) \
  LP1NR(288, XML_DefaultCurrent, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetStartDoctypeDeclHandler(__p0, __p1) \
  LP2NR(186, XML_SetStartDoctypeDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_StartDoctypeDeclHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetFeatureList() \
  LP0(372, const XML_Feature *, XML_GetFeatureList, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ExpatVersion() \
  LP0(342, const XML_LChar *, XML_ExpatVersion, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_GetSpecifiedAttributeCount(__p0) \
  LP1(312, int , XML_GetSpecifiedAttributeCount, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetProcessingInstructionHandler(__p0, __p1) \
  LP2NR(102, XML_SetProcessingInstructionHandler, \
    XML_Parser , __p0, a0, \
    XML_ProcessingInstructionHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetStartCdataSectionHandler(__p0, __p1) \
  LP2NR(114, XML_SetStartCdataSectionHandler, \
    XML_Parser , __p0, a0, \
    XML_StartCdataSectionHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetParamEntityParsing(__p0, __p1) \
  LP2(330, int , XML_SetParamEntityParsing, \
    XML_Parser , __p0, a0, \
    int , __p1, d0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_FreeContentModel(__p0, __p1) \
  LP2NR(396, XML_FreeContentModel, \
    XML_Parser , __p0, a0, \
    XML_Content *, __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetNamespaceDeclHandler(__p0, __p1, __p2) \
  LP3NR(174, XML_SetNamespaceDeclHandler, \
    XML_Parser , __p0, a0, \
    XML_StartNamespaceDeclHandler , __p1, a1, \
    XML_EndNamespaceDeclHandler , __p2, a2, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetDefaultHandler(__p0, __p1) \
  LP2NR(132, XML_SetDefaultHandler, \
    XML_Parser , __p0, a0, \
    XML_DefaultHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_ParserFree(__p0) \
  LP1NR(54, XML_ParserFree, \
    XML_Parser , __p0, a0, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XML_SetDefaultHandlerExpand(__p0, __p1) \
  LP2NR(138, XML_SetDefaultHandlerExpand, \
    XML_Parser , __p0, a0, \
    XML_DefaultHandler , __p1, a1, \
    , EXPAT_BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE_EXPAT_H */
