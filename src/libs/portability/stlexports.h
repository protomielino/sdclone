/***************************************************************************

    file        : stlexports.h
    created     : Fri Jun 18 15:19:34 CET 2010
    copyright   : (C) 2010 Jean-Philippe Meuret
    email       : pouillot@users.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SD_STLEXPORTS_H_
#define _SD_STLEXPORTS_H_

// Macros for exporting STL container types across Windows DLL interfaces
// with MSVC 7 and above compilers.
//
// Thanks to rms@unknownroad.com
// http://www.unknownroad.com/rtfm/VisualStudio/warningC4251.html

#ifndef WIN32

// Of course, nothing to do if not Windows.

#define ExportSTLVector(exDLLMacro, exValueType)
//#define ExportSTLSet(exDLLMacro, exValueType)
//#define ExportSTLMap(exDLLMacro, exKeyType, exValueType)

#else

#define ExportSTLVector(exDLLMacro, exValueType) \
  template class exDLLMacro std::allocator< exValueType >; \
  template class exDLLMacro std::vector<exValueType, \
    std::allocator< exValueType > >;

/* Strongly unrecommended everywhere because of MS STL implementation
   using static data that cause issues across DLL frontiers

#define ExportSTLSet(exDLLMacro, exValueType) \
  template class exDLLMacro std::allocator< exValueType >; \
  template struct exDLLMacro std::less< exValueType >; \
  template class exDLLMacro std::allocator< \
    std::_Tree_nod<std::_Tset_traits<exValueType,std::less<exValueType>, \
    std::allocator<exValueType>,false> > >; \
  template class exDLLMacro std::allocator<  \
    std::_Tree_ptr<std::_Tset_traits<exValueType,std::less<exValueType>, \
    std::allocator<exValueType>,false> > >; \
  template class exDLLMacro std::_Tree_ptr< \
    std::_Tset_traits<exValueType,std::less<exValueType>, \
    std::allocator<exValueType>,false> >; \
  template class exDLLMacro std::_Tree_nod< \
    std::_Tset_traits<exValueType,std::less<exValueType>, \
    std::allocator<exValueType>,false> >; \
  template class exDLLMacro std::_Tree_val< \
    std::_Tset_traits<exValueType,std::less<exValueType>, \
    std::allocator<exValueType>,false> >; \
  template class exDLLMacro std::set< exValueType, std::less< exValueType >, \
    std::allocator< exValueType > >;

#define ExportSTLMap(exDLLMacro, exKeyType, exValueType) \
  template struct exDLLMacro std::pair< exKeyType,exValueType >; \
  template class exDLLMacro std::allocator< \
    std::pair<const exKeyType,exValueType> >; \
  template struct exDLLMacro std::less< exKeyType >; \
  template class exDLLMacro std::allocator< \
    std::_Tree_ptr<std::_Tmap_traits<exKeyType,exValueType,std::less<exKeyType>, \
    std::allocator<std::pair<const exKeyType,exValueType> >,false> > >; \
  template class exDLLMacro std::allocator< \
    std::_Tree_nod<std::_Tmap_traits<exKeyType,exValueType,std::less<exKeyType>, \
    std::allocator<std::pair<const exKeyType,exValueType> >,false> > >; \
  template class exDLLMacro std::_Tree_nod< \
    std::_Tmap_traits<exKeyType,exValueType,std::less<exKeyType>, \
    std::allocator<std::pair<const exKeyType,exValueType> >,false> >; \
  template class exDLLMacro std::_Tree_ptr< \
    std::_Tmap_traits<exKeyType,exValueType,std::less<exKeyType>, \
    std::allocator<std::pair<const exKeyType,exValueType> >,false> >; \
  template class exDLLMacro std::_Tree_val< \
    std::_Tmap_traits<exKeyType,exValueType,std::less<exKeyType>, \
	std::allocator<std::pair<const exKeyType,exValueType> >,false> >; \
  template class exDLLMacro std::map< \
    exKeyType, exValueType, std::less< exKeyType >, \
    std::allocator<std::pair<const exKeyType,exValueType> > >;

*/

#endif // ndef WIN32

#endif // _SD_STLEXPORTS_H_
