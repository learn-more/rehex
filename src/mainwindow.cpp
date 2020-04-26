/* Reverse Engineer's Hex Editor
 * Copyright (C) 2017-2020 Daniel Collins <solemnwarning@solemnwarning.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <exception>
#include <inttypes.h>
#include <limits>
#include <new>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/event.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/aui/auibook.h>
#include <wx/numdlg.h>
#include <wx/sizer.h>

#include "AboutDialog.hpp"
#include "app.hpp"
#include "DiffWindow.hpp"
#include "EditCommentDialog.hpp"
#include "Events.hpp"
#include "mainwindow.hpp"
#include "NumericEntryDialog.hpp"
#include "Palette.hpp"
#include "search.hpp"
#include "SelectRangeDialog.hpp"
#include "ToolPanel.hpp"
#include "util.hpp"

#include "../res/icon16.h"
#include "../res/icon32.h"
#include "../res/icon48.h"
#include "../res/icon64.h"

#ifdef __APPLE__
#include "../res/document_new32.h"
#include "../res/document_open32.h"
#include "../res/document_save32.h"
#include "../res/document_save_as32.h"
#endif

enum {
	ID_BYTES_LINE = 1,
	ID_BYTES_GROUP,
	ID_SHOW_OFFSETS,
	ID_SHOW_ASCII,
	ID_SEARCH_TEXT,
	ID_SEARCH_BSEQ,
	ID_SEARCH_VALUE,
	ID_GOTO_OFFSET,
	ID_OVERWRITE_MODE,
	ID_SAVE_VIEW,
	ID_INLINE_COMMENTS_HIDDEN,
	ID_INLINE_COMMENTS_FULL,
	ID_INLINE_COMMENTS_SHORT,
	ID_INLINE_COMMENTS_INDENT,
	ID_HIGHLIGHT_SELECTION_MATCH,
	ID_HEX_OFFSETS,
	ID_DEC_OFFSETS,
	ID_SELECT_RANGE,
	ID_SYSTEM_PALETTE,
	ID_LIGHT_PALETTE,
	ID_DARK_PALETTE,
	ID_CLOSE_ALL,
	ID_CLOSE_OTHERS,
	ID_GITHUB,
	ID_DONATE,
};

BEGIN_EVENT_TABLE(REHex::MainWindow, wxFrame)
	EVT_CLOSE(REHex::MainWindow::OnWindowClose)
	
	EVT_MENU(wxID_NEW,        REHex::MainWindow::OnNew)
	EVT_MENU(wxID_OPEN,       REHex::MainWindow::OnOpen)
	EVT_MENU(wxID_SAVE,       REHex::MainWindow::OnSave)
	EVT_MENU(wxID_SAVEAS,     REHex::MainWindow::OnSaveAs)
	EVT_MENU(wxID_CLOSE,      REHex::MainWindow::OnClose)
	EVT_MENU(ID_CLOSE_ALL,    REHex::MainWindow::OnCloseAll)
	EVT_MENU(ID_CLOSE_OTHERS, REHex::MainWindow::OnCloseOthers)
	EVT_MENU(wxID_EXIT,       REHex::MainWindow::OnExit)
	
	EVT_MENU(wxID_FILE1, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE2, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE3, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE4, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE5, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE6, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE7, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE8, REHex::MainWindow::OnRecentOpen)
	EVT_MENU(wxID_FILE9, REHex::MainWindow::OnRecentOpen)
	
	EVT_MENU(wxID_UNDO, REHex::MainWindow::OnUndo)
	EVT_MENU(wxID_REDO, REHex::MainWindow::OnRedo)
	
	EVT_MENU(wxID_SELECTALL, REHex::MainWindow::OnSelectAll)
	EVT_MENU(ID_SELECT_RANGE, REHex::MainWindow::OnSelectRange)
	
	EVT_MENU(ID_OVERWRITE_MODE, REHex::MainWindow::OnOverwriteMode)
	
	EVT_MENU(ID_SEARCH_TEXT, REHex::MainWindow::OnSearchText)
	EVT_MENU(ID_SEARCH_BSEQ,  REHex::MainWindow::OnSearchBSeq)
	EVT_MENU(ID_SEARCH_VALUE,  REHex::MainWindow::OnSearchValue)
	
	EVT_MENU(ID_GOTO_OFFSET, REHex::MainWindow::OnGotoOffset)
	
	EVT_MENU(wxID_CUT,   REHex::MainWindow::OnCut)
	EVT_MENU(wxID_COPY,  REHex::MainWindow::OnCopy)
	EVT_MENU(wxID_PASTE, REHex::MainWindow::OnPaste)
	
	EVT_MENU(ID_BYTES_LINE,   REHex::MainWindow::OnSetBytesPerLine)
	EVT_MENU(ID_BYTES_GROUP,  REHex::MainWindow::OnSetBytesPerGroup)
	EVT_MENU(ID_SHOW_OFFSETS, REHex::MainWindow::OnShowOffsets)
	EVT_MENU(ID_SHOW_ASCII,   REHex::MainWindow::OnShowASCII)
	EVT_MENU(ID_SAVE_VIEW,    REHex::MainWindow::OnSaveView)
	
	EVT_MENU(ID_INLINE_COMMENTS_HIDDEN, REHex::MainWindow::OnInlineCommentsMode)
	EVT_MENU(ID_INLINE_COMMENTS_FULL,   REHex::MainWindow::OnInlineCommentsMode)
	EVT_MENU(ID_INLINE_COMMENTS_SHORT,  REHex::MainWindow::OnInlineCommentsMode)
	EVT_MENU(ID_INLINE_COMMENTS_INDENT, REHex::MainWindow::OnInlineCommentsMode)
	
	EVT_MENU(ID_HIGHLIGHT_SELECTION_MATCH, REHex::MainWindow::OnHighlightSelectionMatch)
	
	EVT_MENU(ID_SYSTEM_PALETTE, REHex::MainWindow::OnPalette)
	EVT_MENU(ID_LIGHT_PALETTE,  REHex::MainWindow::OnPalette)
	EVT_MENU(ID_DARK_PALETTE,   REHex::MainWindow::OnPalette)
	
	EVT_MENU(ID_HEX_OFFSETS,   REHex::MainWindow::OnHexOffsets)
	EVT_MENU(ID_DEC_OFFSETS,   REHex::MainWindow::OnDecOffsets)
	
	EVT_MENU(ID_GITHUB,  REHex::MainWindow::OnGithub)
	EVT_MENU(ID_DONATE,  REHex::MainWindow::OnDonate)
	EVT_MENU(wxID_ABOUT, REHex::MainWindow::OnAbout)
	
	EVT_AUINOTEBOOK_PAGE_CHANGED(  wxID_ANY, REHex::MainWindow::OnDocumentChange)
	EVT_AUINOTEBOOK_PAGE_CLOSE(    wxID_ANY, REHex::MainWindow::OnDocumentClose)
	EVT_AUINOTEBOOK_PAGE_CLOSED(   wxID_ANY, REHex::MainWindow::OnDocumentClosed)
	EVT_AUINOTEBOOK_TAB_RIGHT_DOWN(wxID_ANY, REHex::MainWindow::OnDocumentMenu)
	
	EVT_CURSORUPDATE(wxID_ANY, REHex::MainWindow::OnCursorUpdate)
	
	EVT_COMMAND(wxID_ANY, REHex::EV_SELECTION_CHANGED, REHex::MainWindow::OnSelectionChange)
	EVT_COMMAND(wxID_ANY, REHex::EV_INSERT_TOGGLED,    REHex::MainWindow::OnInsertToggle)
	EVT_COMMAND(wxID_ANY, REHex::EV_UNDO_UPDATE,       REHex::MainWindow::OnUndoUpdate)
	EVT_COMMAND(wxID_ANY, REHex::EV_BECAME_DIRTY,      REHex::MainWindow::OnBecameDirty)
	EVT_COMMAND(wxID_ANY, REHex::EV_BECAME_CLEAN,      REHex::MainWindow::OnBecameClean)
END_EVENT_TABLE()

/* Is the given byte a printable 7-bit ASCII character? */
static bool isasciiprint(int c)
{
	return (c >= ' ' && c <= '~');
}

/* Is the given value a 7-bit ASCII character representing a hex digit? */
static bool isasciihex(int c)
{
	return (c >= '0' && c <= '9')
		|| (c >= 'A' && c <= 'F')
		|| (c >= 'a' && c <= 'f');
}

REHex::MainWindow::MainWindow():
	wxFrame(NULL, wxID_ANY, "Reverse Engineers' Hex Editor", wxDefaultPosition, wxSize(740, 540))
{
	file_menu = new wxMenu;
	recent_files_menu = new wxMenu;
	
	file_menu->Append(wxID_NEW,    "&New\tCtrl-N");
	file_menu->Append(wxID_OPEN,   "&Open\tCtrl-O");
	file_menu->AppendSubMenu(recent_files_menu, "Open &Recent");
	file_menu->Append(wxID_SAVE,   "&Save\tCtrl-S");
	file_menu->Append(wxID_SAVEAS, "&Save As");
	file_menu->AppendSeparator();
	file_menu->Append(wxID_CLOSE,  "&Close\tCtrl-W");
	file_menu->Append(ID_CLOSE_ALL, "Close All");
	file_menu->Append(ID_CLOSE_OTHERS, "Close Others");
	file_menu->AppendSeparator();
	file_menu->Append(wxID_EXIT,   "&Exit");
	
	edit_menu = new wxMenu;
	
	edit_menu->Append(wxID_UNDO, "&Undo\tCtrl-Z");
	edit_menu->Append(wxID_REDO, "&Redo\tCtrl-Shift-Z");
	
	edit_menu->AppendSeparator();
	
	edit_menu->Append(wxID_SELECTALL, "Select &All\tCtrl-A");
	edit_menu->Append(ID_SELECT_RANGE, "Select range...");
	
	edit_menu->AppendSeparator();
	
	#ifdef __APPLE__
	edit_menu->AppendCheckItem(ID_OVERWRITE_MODE, "Overwrite mode");
	#else
	edit_menu->AppendCheckItem(ID_OVERWRITE_MODE, "Overwrite mode\tIns");
	#endif
	
	edit_menu->AppendSeparator();
	
	edit_menu->Append(ID_SEARCH_TEXT,  "Search for text...");
	edit_menu->Append(ID_SEARCH_BSEQ,  "Search for byte sequence...");
	edit_menu->Append(ID_SEARCH_VALUE, "Search for value...");
	
	edit_menu->AppendSeparator();
	
	edit_menu->Append(ID_GOTO_OFFSET, "Jump to offset...\tCtrl-G");
	
	edit_menu->AppendSeparator();
	
	edit_menu->Append(wxID_CUT,   "Cu&t\tCtrl-X");
	edit_menu->Append(wxID_COPY,  "&Copy\tCtrl-C");
	edit_menu->Append(wxID_PASTE, "&Paste\tCtrl-V");
	
	view_menu = new wxMenu;
	
	view_menu->Append(ID_BYTES_LINE,  "Set bytes per line");
	view_menu->Append(ID_BYTES_GROUP, "Set bytes per group");
	view_menu->AppendCheckItem(ID_SHOW_OFFSETS, "Show offsets");
	view_menu->AppendCheckItem(ID_SHOW_ASCII, "Show ASCII");
	
	inline_comments_menu = new wxMenu;
	view_menu->AppendSubMenu(inline_comments_menu, "Inline comments");
	
	view_menu->AppendCheckItem(ID_HIGHLIGHT_SELECTION_MATCH, "Highlight data matching selection");
	
	inline_comments_menu->AppendRadioItem(ID_INLINE_COMMENTS_HIDDEN, "Hidden");
	inline_comments_menu->AppendRadioItem(ID_INLINE_COMMENTS_SHORT,  "Short");
	inline_comments_menu->AppendRadioItem(ID_INLINE_COMMENTS_FULL,   "Full");
	inline_comments_menu->AppendSeparator();
	inline_comments_menu->AppendCheckItem(ID_INLINE_COMMENTS_INDENT, "Nest comments");
	
	tool_panels_menu = new wxMenu;
	view_menu->AppendSubMenu(tool_panels_menu, "Tool panels");
	
	for(auto i = ToolPanelRegistry::begin(); i != ToolPanelRegistry::end(); ++i)
	{
		const ToolPanelRegistration *tpr = i->second;
		wxMenuItem *itm = tool_panels_menu->AppendCheckItem(wxID_ANY, tpr->label);
		
		Bind(wxEVT_MENU, [this, tpr](wxCommandEvent &event)
		{
			OnShowToolPanel(event, tpr);
		}, itm->GetId(), itm->GetId());
		
		tool_panel_name_to_tpm_id[tpr->name] = itm->GetId();
	}
	
	view_menu->AppendSeparator();
	
	view_menu->AppendRadioItem(ID_HEX_OFFSETS, "Display offsets in hexadecimal");
	view_menu->AppendRadioItem(ID_DEC_OFFSETS, "Display offsets in decimal");
	
	view_menu->AppendSeparator();
	
	wxMenu *palette_menu = new wxMenu;
	view_menu->AppendSubMenu(palette_menu, "Colour scheme");
	
	palette_menu->AppendRadioItem(ID_SYSTEM_PALETTE, "System");
	palette_menu->AppendRadioItem(ID_LIGHT_PALETTE,  "Light");
	palette_menu->AppendRadioItem(ID_DARK_PALETTE,   "Dark");
	
	std::string palette_name = active_palette->get_name();
	if(palette_name == "light")
	{
		palette_menu->Check(ID_LIGHT_PALETTE, true);
	}
	else if(palette_name == "dark")
	{
		palette_menu->Check(ID_DARK_PALETTE, true);
	}
	else /* if(palette_name == "system") */
	{
		palette_menu->Check(ID_SYSTEM_PALETTE, true);
	}
	
	view_menu->AppendSeparator();
	
	view_menu->Append(ID_SAVE_VIEW, "Save current view as default");
	
	wxMenu *help_menu = new wxMenu;
	
	help_menu->Append(ID_GITHUB, "Visit &Github page");
	help_menu->Append(ID_DONATE, "Donate with &Paypal");
	help_menu->Append(wxID_ABOUT, "&About");
	
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, "&File");
	menu_bar->Append(edit_menu, "&Edit");
	menu_bar->Append(view_menu,  "&View");
	menu_bar->Append(help_menu, "&Help");
	
	SetMenuBar(menu_bar);
	
	wxGetApp().recent_files->UseMenu(recent_files_menu);
	wxGetApp().recent_files->AddFilesToMenu(recent_files_menu);
	
	wxToolBar *toolbar = CreateToolBar();
	wxArtProvider artp;
	
	/* Toolbar icons are expected to be 32x32 on OS X. wxWidgets ships 16x16 and 24x24 Tango
	 * icons and scales them as needed, which produces blurry 32x32 images. So on OS X, we
	 * embed 32x32 versions instead.
	*/
	
	#ifdef __APPLE__
	toolbar->AddTool(wxID_NEW,    "New",     wxBITMAP_PNG_FROM_DATA(document_new32));
	toolbar->AddTool(wxID_OPEN,   "Open",    wxBITMAP_PNG_FROM_DATA(document_open32));
	toolbar->AddTool(wxID_SAVE,   "Save",    wxBITMAP_PNG_FROM_DATA(document_save32));
	toolbar->AddTool(wxID_SAVEAS, "Save As", wxBITMAP_PNG_FROM_DATA(document_save_as32));
	#else
	toolbar->AddTool(wxID_NEW,    "New",     artp.GetBitmap(wxART_NEW,          wxART_TOOLBAR));
	toolbar->AddTool(wxID_OPEN,   "Open",    artp.GetBitmap(wxART_FILE_OPEN,    wxART_TOOLBAR));
	toolbar->AddTool(wxID_SAVE,   "Save",    artp.GetBitmap(wxART_FILE_SAVE,    wxART_TOOLBAR));
	toolbar->AddTool(wxID_SAVEAS, "Save As", artp.GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR));
	#endif
	
	toolbar->Realize();
	
	notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		(wxAUI_NB_TOP | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_CLOSE_ON_ALL_TABS));
	
	notebook_dirty_bitmap = artp.GetBitmap(wxART_FILE_SAVE, wxART_MENU);
	assert(!notebook_dirty_bitmap.IsSameAs(wxNullBitmap));
	
	CreateStatusBar(3);
	
	SetDropTarget(new DropTarget(this));
	
	/* TODO: Construct a single wxIconBundle instance somewhere. */
	
	wxIconBundle icons;
	
	{
		wxBitmap b16 = wxBITMAP_PNG_FROM_DATA(icon16);
		wxIcon i16;
		i16.CopyFromBitmap(b16);
		icons.AddIcon(i16);
		
		wxBitmap b32 = wxBITMAP_PNG_FROM_DATA(icon32);
		wxIcon i32;
		i32.CopyFromBitmap(b32);
		icons.AddIcon(i32);
		
		wxBitmap b48 = wxBITMAP_PNG_FROM_DATA(icon48);
		wxIcon i48;
		i48.CopyFromBitmap(b48);
		icons.AddIcon(i48);
		
		wxBitmap b64 = wxBITMAP_PNG_FROM_DATA(icon64);
		wxIcon i64;
		i64.CopyFromBitmap(b64);
		icons.AddIcon(i64);
	}
	
	SetIcons(icons);
}

REHex::MainWindow::~MainWindow()
{
	wxGetApp().recent_files->RemoveMenu(recent_files_menu);
}

void REHex::MainWindow::new_file()
{
	Tab *tab = new Tab(notebook);
	notebook->AddPage(tab, tab->doc->get_title(), true);
	tab->doc_ctrl->SetFocus();
}

void REHex::MainWindow::open_file(const std::string &filename)
{
	Tab *tab;
	try {
		tab = new Tab(notebook, filename);
	}
	catch(const std::exception &e)
	{
		wxMessageBox(
			std::string("Error opening ") + filename + ":\n" + e.what(),
			"Error", wxICON_ERROR, this);
		return;
	}
	
	/* Discard default "Untitled" tab if not modified. */
	if(notebook->GetPageCount() == 1)
	{
		wxWindow *page = notebook->GetPage(0);
		assert(page != NULL);
		
		auto page_tab = dynamic_cast<REHex::MainWindow::Tab*>(page);
		assert(page_tab != NULL);
		
		if(page_tab->doc->get_filename() == "" && !page_tab->doc->is_dirty())
		{
			notebook->DeletePage(0);
		}
	}
	
	wxFileName wxfn(filename);
	wxfn.MakeAbsolute();
	wxGetApp().recent_files->AddFileToHistory(wxfn.GetFullPath());
	
	notebook->AddPage(tab, tab->doc->get_title(), true);
	tab->doc_ctrl->SetFocus();
}

void REHex::MainWindow::OnWindowClose(wxCloseEvent &event)
{
	if(!unsaved_confirm())
	{
		/* Stop the window from being closed. */
		event.Veto();
		return;
	}
	
	/* Base implementation will deal with cleaning up the window. */
	event.Skip();
}

void REHex::MainWindow::OnNew(wxCommandEvent &event)
{
	new_file();
}

void REHex::MainWindow::OnOpen(wxCommandEvent &event)
{
	std::string dir;
	std::string doc_filename = active_document()->get_filename();
	
	if(doc_filename != "")
	{
		wxFileName wxfn(doc_filename);
		wxfn.MakeAbsolute();
		
		dir = wxfn.GetPath();
	}
	else{
		dir = wxGetApp().get_last_directory();
	}
	
	wxFileDialog openFileDialog(this, "Open File", dir, "", "", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if(openFileDialog.ShowModal() == wxID_CANCEL)
		return;
	
	std::string filename = openFileDialog.GetPath().ToStdString();
	
	{
		wxFileName wxfn(filename);
		wxString dirname = wxfn.GetPath();
		
		wxGetApp().set_last_directory(dirname.ToStdString());
	}
	
	open_file(filename);
}

void REHex::MainWindow::OnRecentOpen(wxCommandEvent &event)
{
	wxFileHistory *recent_files = wxGetApp().recent_files;
	wxString file = recent_files->GetHistoryFile(event.GetId() - recent_files->GetBaseId());
	
	open_file(file.ToStdString());
}

void REHex::MainWindow::OnSave(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	if(tab->doc->get_filename() == "")
	{
		OnSaveAs(event);
		return;
	}
	
	try {
		tab->doc->save();
		_update_dirty(tab->doc);
	}
	catch(const std::exception &e)
	{
		wxMessageBox(
			std::string("Error saving ") + tab->doc->get_title() + ":\n" + e.what(),
			"Error", wxICON_ERROR, this);
		return;
	}
}

void REHex::MainWindow::OnSaveAs(wxCommandEvent &event)
{
	std::string dir, name;
	std::string doc_filename = active_document()->get_filename();
	
	if(doc_filename != "")
	{
		wxFileName wxfn(doc_filename);
		wxfn.MakeAbsolute();
		
		dir  = wxfn.GetPath();
		name = wxfn.GetFullName();
	}
	else{
		dir  = wxGetApp().get_last_directory();
		name = "";
	}
	
	wxFileDialog saveFileDialog(this, "Save As", dir, name, "", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if(saveFileDialog.ShowModal() == wxID_CANCEL)
		return;
	
	std::string filename = saveFileDialog.GetPath().ToStdString();
	
	{
		wxFileName wxfn(filename);
		wxString dirname = wxfn.GetPath();
		
		wxGetApp().set_last_directory(dirname.ToStdString());
	}
	
	Tab *tab = active_tab();
	
	try {
		tab->doc->save(filename);
		_update_dirty(tab->doc);
	}
	catch(const std::exception &e)
	{
		wxMessageBox(
			std::string("Error saving ") + tab->doc->get_title() + ":\n" + e.what(),
			"Error", wxICON_ERROR, this);
		return;
	}
	
	notebook->SetPageText(notebook->GetSelection(), tab->doc->get_title());
}

void REHex::MainWindow::OnClose(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	close_tab(tab);
}

void REHex::MainWindow::OnCloseAll(wxCommandEvent &event)
{
	close_all_tabs();
}

void REHex::MainWindow::OnCloseOthers(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	close_other_tabs(tab);
}

void REHex::MainWindow::OnExit(wxCommandEvent &event)
{
	Close();
}

void REHex::MainWindow::OnSearchText(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	REHex::Search::Text *sd = new REHex::Search::Text(tab, *(tab->doc));
	sd->Show(true);
	
	tab->search_dialog_register(sd);
}

void REHex::MainWindow::OnSearchBSeq(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	REHex::Search::ByteSequence *sd = new REHex::Search::ByteSequence(tab, *(tab->doc));
	sd->Show(true);
	
	tab->search_dialog_register(sd);
}

void REHex::MainWindow::OnSearchValue(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	REHex::Search::Value *sd = new REHex::Search::Value(tab, *(tab->doc));
	sd->Show(true);
	
	tab->search_dialog_register(sd);
}

void REHex::MainWindow::OnGotoOffset(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	
	off_t current_pos = tab->doc->get_cursor_position();
	off_t max_pos     = tab->doc->buffer_length() - !tab->doc_ctrl->get_insert_mode();
	
	REHex::NumericEntryDialog<off_t> ni(this,
		"Jump to offset",
		"Prefix offset with -/+ to jump relative to current cursor position",
		current_pos, 0, max_pos, current_pos);
	
	int rc = ni.ShowModal();
	if(rc == wxID_OK)
	{
		tab->doc->set_cursor_position(ni.GetValue());
	}
}

void REHex::MainWindow::OnCut(wxCommandEvent &event)
{
	_clipboard_copy(true);
}

void REHex::MainWindow::OnCopy(wxCommandEvent &event)
{
	_clipboard_copy(false);
}

void REHex::MainWindow::OnPaste(wxCommandEvent &event)
{
	REHex::Document *doc = active_document();
	
	ClipboardGuard cg;
	if(cg)
	{
		if(wxTheClipboard->IsSupported(CommentsDataObject::format))
		{
			CommentsDataObject data;
			wxTheClipboard->GetData(data);
			
			auto clipboard_comments = data.get_comments();
			
			doc->handle_paste(clipboard_comments);
		}
		else if(wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			
			try {
				doc->handle_paste(data.GetText().ToStdString());
			}
			catch(const std::exception &e)
			{
				wxMessageBox(e.what(), "Error", (wxOK | wxICON_ERROR), this);
			}
		}
	}
}

void REHex::MainWindow::OnUndo(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	tab->doc->undo();
}

void REHex::MainWindow::OnRedo(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	tab->doc->redo();
}

void REHex::MainWindow::OnSelectAll(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	tab->doc->set_selection(0, tab->doc->buffer_length());
}

void REHex::MainWindow::OnSelectRange(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	
	REHex::SelectRangeDialog srd(this, *(tab->doc), *(tab->doc_ctrl));
	srd.ShowModal();
}

void REHex::MainWindow::OnOverwriteMode(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	tab->doc_ctrl->set_insert_mode(!event.IsChecked());
}

void REHex::MainWindow::OnSetBytesPerLine(wxCommandEvent &event)
{
	/* There are rendering/performance issues with very large values here, which we just bypass
	 * with a nice arbitrary limit for now.
	*/
	const int MAX_BYTES_PER_LINE = 128;
	
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	/* TODO: Make a dialog with an explicit "auto" radio choice? */
	int new_value = wxGetNumberFromUser(
		"Number of bytes to show on each line\n(0 fits to the window width)",
		"Bytes",
		"Set bytes per line",
		tab->doc_ctrl->get_bytes_per_line(),
		0,
		MAX_BYTES_PER_LINE,
		this);
	
	/* We get a negative value if the user cancels. */
	if(new_value >= 0)
	{
		tab->doc_ctrl->set_bytes_per_line(new_value);
	}
}

void REHex::MainWindow::OnSetBytesPerGroup(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	int new_value = wxGetNumberFromUser(
		"Number of bytes to group",
		"Bytes",
		"Set bytes per group",
		tab->doc_ctrl->get_bytes_per_group(),
		1,
		std::numeric_limits<int>::max(),
		this);
	
	/* We get a negative value if the user cancels. */
	if(new_value >= 0)
	{
		tab->doc_ctrl->set_bytes_per_group(new_value);
	}
}

void REHex::MainWindow::OnShowOffsets(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	tab->doc_ctrl->set_show_offsets(event.IsChecked());
}

void REHex::MainWindow::OnShowASCII(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	tab->doc_ctrl->set_show_ascii(event.IsChecked());
}

void REHex::MainWindow::OnInlineCommentsMode(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	if(inline_comments_menu->IsChecked(ID_INLINE_COMMENTS_HIDDEN))
	{
		tab->inline_comment_mode = ICM_HIDDEN;
		tab->repopulate_regions();
		
		inline_comments_menu->Enable(ID_INLINE_COMMENTS_INDENT, false);
	}
	else if(inline_comments_menu->IsChecked(ID_INLINE_COMMENTS_FULL))
	{
		tab->inline_comment_mode = inline_comments_menu->IsChecked(ID_INLINE_COMMENTS_INDENT) ? ICM_FULL_INDENT : ICM_FULL;
		tab->repopulate_regions();
		
		inline_comments_menu->Enable(ID_INLINE_COMMENTS_INDENT, true);
	}
	else if(inline_comments_menu->IsChecked(ID_INLINE_COMMENTS_SHORT))
	{
		tab->inline_comment_mode = inline_comments_menu->IsChecked(ID_INLINE_COMMENTS_INDENT) ? ICM_SHORT_INDENT : ICM_SHORT;
		tab->repopulate_regions();
		
		inline_comments_menu->Enable(ID_INLINE_COMMENTS_INDENT, true);
	}
}

void REHex::MainWindow::OnHighlightSelectionMatch(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	tab->doc_ctrl->set_highlight_selection_match(event.IsChecked());
}

void REHex::MainWindow::OnShowToolPanel(wxCommandEvent &event, const REHex::ToolPanelRegistration *tpr)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	if(event.IsChecked())
	{
		tab->tool_create(tpr->name, true);
	}
	else{
		tab->tool_destroy(tpr->name);
	}
}

void REHex::MainWindow::OnPalette(wxCommandEvent &event)
{
	delete active_palette;
	
	switch(event.GetId())
	{
		case ID_SYSTEM_PALETTE:
			active_palette = Palette::create_system_palette();
			break;
			
		case ID_LIGHT_PALETTE:
			active_palette = Palette::create_light_palette();
			break;
			
		case ID_DARK_PALETTE:
			active_palette = Palette::create_dark_palette();
			break;
			
		default:
			abort();
	}
	
	Refresh();
}

void REHex::MainWindow::OnHexOffsets(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	
	tab->doc_ctrl->set_offset_display_base(OFFSET_BASE_HEX);
	
	_update_status_offset(tab->doc_ctrl);
	_update_status_selection(tab->doc_ctrl);
}

void REHex::MainWindow::OnDecOffsets(wxCommandEvent &event)
{
	Tab *tab = active_tab();
	
	tab->doc_ctrl->set_offset_display_base(OFFSET_BASE_DEC);
	
	_update_status_offset(tab->doc_ctrl);
	_update_status_selection(tab->doc_ctrl);
}

void REHex::MainWindow::OnSaveView(wxCommandEvent &event)
{
	wxConfig *config = wxGetApp().config;
	
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	tab->save_view(config);
}

void REHex::MainWindow::OnGithub(wxCommandEvent &event)
{
	wxLaunchDefaultBrowser("https://github.com/solemnwarning/rehex/");
}

void REHex::MainWindow::OnDonate(wxCommandEvent &event)
{
	wxLaunchDefaultBrowser("https://www.solemnwarning.net/rehex/donate");
}

void REHex::MainWindow::OnAbout(wxCommandEvent &event)
{
	REHex::AboutDialog about(this);
	about.ShowModal();
}

void REHex::MainWindow::OnDocumentChange(wxAuiNotebookEvent& event)
{
	int old_page_id = event.GetOldSelection();
	if(old_page_id != wxNOT_FOUND && old_page_id < (int)(notebook->GetPageCount()))
	{
		/* Hide any search dialogs attached to previous tab. */
		
		wxWindow *old_page = notebook->GetPage(old_page_id);
		assert(old_page != NULL);
		
		auto old_tab = dynamic_cast<REHex::MainWindow::Tab*>(old_page);
		assert(old_tab != NULL);
		
		for(auto sdi = old_tab->search_dialogs.begin(); sdi != old_tab->search_dialogs.end(); ++sdi)
		{
			(*sdi)->Hide();
		}
	}
	
	Tab *tab = active_tab();
	
	edit_menu->Check(ID_OVERWRITE_MODE, !tab->doc_ctrl->get_insert_mode());
	view_menu->Check(ID_SHOW_OFFSETS, tab->doc_ctrl->get_show_offsets());
	view_menu->Check(ID_SHOW_ASCII,   tab->doc_ctrl->get_show_ascii());
	
	OffsetBase offset_display_base = tab->doc_ctrl->get_offset_display_base();
	switch(offset_display_base)
	{
		case OFFSET_BASE_HEX:
			view_menu->Check(ID_HEX_OFFSETS, true);
			break;
			
		case OFFSET_BASE_DEC:
			view_menu->Check(ID_DEC_OFFSETS, true);
			break;
	}
	
	switch(tab->inline_comment_mode)
	{
		case ICM_HIDDEN:
			inline_comments_menu->Check(ID_INLINE_COMMENTS_HIDDEN, true);
			inline_comments_menu->Enable(ID_INLINE_COMMENTS_INDENT, false);
			break;
			
		case ICM_FULL:
		case ICM_FULL_INDENT:
			inline_comments_menu->Check(ID_INLINE_COMMENTS_FULL, true);
			inline_comments_menu->Check(ID_INLINE_COMMENTS_INDENT, (tab->inline_comment_mode == ICM_FULL_INDENT));
			inline_comments_menu->Enable(ID_INLINE_COMMENTS_INDENT, true);
			break;
			
		case ICM_SHORT:
		case ICM_SHORT_INDENT:
			inline_comments_menu->Check(ID_INLINE_COMMENTS_SHORT, true);
			inline_comments_menu->Check(ID_INLINE_COMMENTS_INDENT, (tab->inline_comment_mode == ICM_SHORT_INDENT));
			inline_comments_menu->Enable(ID_INLINE_COMMENTS_INDENT, true);
			break;
	};
	
	view_menu->Check(ID_HIGHLIGHT_SELECTION_MATCH, tab->doc_ctrl->get_highlight_selection_match());
	
	for(auto i = ToolPanelRegistry::begin(); i != ToolPanelRegistry::end(); ++i)
	{
		const ToolPanelRegistration *tpr = i->second;
		
		int menu_id = tool_panel_name_to_tpm_id[tpr->name];
		bool active = tab->tool_active(tpr->name);
		
		tool_panels_menu->Check(menu_id, active);
	}
	
	_update_status_offset(tab->doc_ctrl);
	_update_status_selection(tab->doc_ctrl);
	_update_status_mode(tab->doc_ctrl);
	_update_undo(tab->doc);
	_update_dirty(tab->doc);
	
	/* Show any search dialogs attached to this tab. */
	for(auto sdi = tab->search_dialogs.begin(); sdi != tab->search_dialogs.end(); ++sdi)
	{
		(*sdi)->ShowWithoutActivating();
	}
}

void REHex::MainWindow::OnDocumentClose(wxAuiNotebookEvent& event)
{
	wxWindow *page = notebook->GetPage(event.GetSelection());
	assert(page != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(page);
	assert(tab != NULL);
	
	if(tab->doc->is_dirty())
	{
		wxMessageDialog confirm(this, (wxString("The file ") + tab->doc->get_title() + " has unsaved changes.\nClose anyway?"), "Unsaved changes",
			(wxYES | wxNO | wxCENTER));
		
		int response = confirm.ShowModal();
		
		if(response == wxID_NO)
		{
			event.Veto();
		}
	}
}

void REHex::MainWindow::OnDocumentClosed(wxAuiNotebookEvent &event)
{
	/* Create a new tab if the only one was just closed. */
	if(notebook->GetPageCount() == 0)
	{
		ProcessCommand(wxID_NEW);
	}
}

void REHex::MainWindow::OnDocumentMenu(wxAuiNotebookEvent &event)
{
	int tab_idx = event.GetSelection();
	
	wxWindow *tab_page = notebook->GetPage(tab_idx);
	assert(tab_page != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(tab_page);
	assert(tab != NULL);
	
	std::string filename = tab->doc->get_filename();
	
	wxMenu menu;
	
	wxMenuItem *open_dir = menu.Append(wxID_ANY, "Open Folder");
	open_dir->Enable(filename != "");
	
	menu.Bind(wxEVT_MENU, [&filename](wxCommandEvent &event)
	{
		REHex::file_manager_show_file(filename);
	}, open_dir->GetId(), open_dir->GetId());
	
	menu.AppendSeparator();
	
	wxMenuItem *close = menu.Append(wxID_ANY, "Close");
	menu.Bind(wxEVT_MENU, [this, tab](wxCommandEvent &event)
	{
		close_tab(tab);
	}, close->GetId(), close->GetId());
	
	wxMenuItem *close_all = menu.Append(wxID_ANY, "Close All");
	menu.Bind(wxEVT_MENU, [this](wxCommandEvent &event)
	{
		close_all_tabs();
	}, close_all->GetId(), close_all->GetId());
	
	wxMenuItem *close_others = menu.Append(wxID_ANY, "Close Others");
	menu.Bind(wxEVT_MENU, [this, tab](wxCommandEvent &event)
	{
		close_other_tabs(tab);
	}, close_others->GetId(), close_others->GetId());
	
	PopupMenu(&menu);
}

void REHex::MainWindow::OnCursorUpdate(CursorUpdateEvent &event)
{
	Tab *active_tab = this->active_tab();
	
	wxObject *event_src = event.GetEventObject();
	
	if(event_src == active_tab->doc)
	{
		/* Only update the status bar if the event originated from the
		 * active document.
		*/
		_update_status_offset(active_tab->doc_ctrl);
	}
	
	event.Skip();
}

void REHex::MainWindow::OnSelectionChange(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto active_tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(active_tab != NULL);
	
	DocumentCtrl *doc_ctrl = dynamic_cast<REHex::DocumentCtrl*>(event.GetEventObject());
	assert(doc_ctrl != NULL);
	
	if(doc_ctrl == active_tab->doc_ctrl)
	{
		/* Only update the status bar if the event originated from the
		 * active document.
		*/
		_update_status_selection(doc_ctrl);
	}
}

void REHex::MainWindow::OnInsertToggle(wxCommandEvent &event)
{
	Tab *active_tab = this->active_tab();
	
	DocumentCtrl *event_src = dynamic_cast<DocumentCtrl*>(event.GetEventObject());
	assert(event_src != NULL);
	
	if(event_src == active_tab->doc_ctrl)
	{
		/* Only update the status bar if the event originated from the
		 * active document.
		*/
		
		_update_status_mode(active_tab->doc_ctrl);
		edit_menu->Check(ID_OVERWRITE_MODE, !active_tab->doc_ctrl->get_insert_mode());
	}
}

void REHex::MainWindow::OnUndoUpdate(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	auto doc = dynamic_cast<REHex::Document*>(event.GetEventObject());
	assert(doc != NULL);
	
	if(doc == tab->doc)
	{
		/* Only update the menu if the event originated from the active document. */
		_update_undo(tab->doc);
	}
}

void REHex::MainWindow::OnBecameDirty(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	auto doc = dynamic_cast<REHex::Document*>(event.GetEventObject());
	assert(doc != NULL);
	
	if(doc == tab->doc)
	{
		/* Only update the window if the event originated from the active document. */
		_update_dirty(tab->doc);
	}
}

void REHex::MainWindow::OnBecameClean(wxCommandEvent &event)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	auto doc = dynamic_cast<REHex::Document*>(event.GetEventObject());
	assert(doc != NULL);
	
	if(doc == tab->doc)
	{
		/* Only update the window if the event originated from the active document. */
		_update_dirty(tab->doc);
	}
}

REHex::MainWindow::Tab *REHex::MainWindow::active_tab()
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	return tab;
}

REHex::Document *REHex::MainWindow::active_document()
{
	return active_tab()->doc;
}

void REHex::MainWindow::_update_status_offset(REHex::DocumentCtrl *doc_ctrl)
{
	off_t off = doc_ctrl->get_cursor_position();
	
	std::string off_text = format_offset(off, doc_ctrl->get_offset_display_base());
	
	SetStatusText(off_text, 0);
}

void REHex::MainWindow::_update_status_selection(REHex::DocumentCtrl *doc_ctrl)
{
	std::pair<off_t,off_t> selection = doc_ctrl->get_selection();
	
	off_t selection_off    = selection.first;
	off_t selection_length = selection.second;
	
	if(selection_length > 0)
	{
		off_t selection_end = (selection_off + selection_length) - 1;
		
		std::string from_text = format_offset(selection_off, doc_ctrl->get_offset_display_base(), selection_end);
		std::string to_text   = format_offset(selection_end, doc_ctrl->get_offset_display_base(), selection_end);
		
		char buf[64];
		snprintf(buf, sizeof(buf), "Selection: %s - %s (%u bytes)",
			from_text.c_str(),
			to_text.c_str(),
			
			(unsigned int)(selection_length));
		
		SetStatusText(buf, 1);
	}
	else{
		SetStatusText("", 1);
	}
}

void REHex::MainWindow::_update_status_mode(REHex::DocumentCtrl *doc_ctrl)
{
	if(doc_ctrl->get_insert_mode())
	{
		SetStatusText("Mode: Insert", 2);
	}
	else{
		SetStatusText("Mode: Overwrite", 2);
	}
}

void REHex::MainWindow::_update_undo(REHex::Document *doc)
{
	const char *undo_desc = doc->undo_desc();
	if(undo_desc != NULL)
	{
		char label[64];
		snprintf(label, sizeof(label), "&Undo %s\tCtrl-Z", undo_desc);
		
		edit_menu->SetLabel(wxID_UNDO, label);
		edit_menu->Enable(wxID_UNDO, true);
	}
	else{
		edit_menu->SetLabel(wxID_UNDO, "&Undo\tCtrl-Z");
		edit_menu->Enable(wxID_UNDO, false);
	}
	
	const char *redo_desc = doc->redo_desc();
	if(redo_desc != NULL)
	{
		char label[64];
		snprintf(label, sizeof(label), "&Redo %s\tCtrl-Shift-Z", redo_desc);
		
		edit_menu->SetLabel(wxID_REDO, label);
		edit_menu->Enable(wxID_REDO, true);
	}
	else{
		edit_menu->SetLabel(wxID_REDO, "&Redo\tCtrl-Shift-Z");
		edit_menu->Enable(wxID_REDO, false);
	}
}

void REHex::MainWindow::_update_dirty(REHex::Document *doc)
{
	bool dirty = doc->is_dirty();
	bool has_file = (doc->get_filename() != "");
	
	bool enable_save = dirty || !has_file;
	
	SetTitle((dirty ? "[UNSAVED] " : "") + doc->get_title() + " - Reverse Engineers' Hex Editor");
	
	file_menu->Enable(wxID_SAVE,   enable_save);
	
	wxToolBar *toolbar = GetToolBar();
	toolbar->EnableTool(wxID_SAVE,   enable_save);
	
	notebook->SetPageBitmap(notebook->GetSelection(), (dirty ? notebook_dirty_bitmap : wxNullBitmap));
}

void REHex::MainWindow::_clipboard_copy(bool cut)
{
	wxWindow *cpage = notebook->GetCurrentPage();
	assert(cpage != NULL);
	
	auto tab = dynamic_cast<REHex::MainWindow::Tab*>(cpage);
	assert(tab != NULL);
	
	/* Warn the user this might be a bad idea before dumping silly amounts
	 * of data (>16MiB) into the clipboard.
	*/
	
	static size_t COPY_MAX_SOFT = 16777216;
	size_t upper_limit = tab->doc->copy_upper_limit();
	
	if(upper_limit > COPY_MAX_SOFT)
	{
		char msg[128];
		snprintf(msg, sizeof(msg),
			"You are about to copy %uMB into the clipboard.\n"
			"This may take a long time and/or crash some applications.",
			(unsigned)(upper_limit / 1000000));
		
		int result = wxMessageBox(msg, "Warning", (wxOK | wxCANCEL | wxICON_EXCLAMATION), this);
		if(result != wxOK)
		{
			return;
		}
	}
	
	wxTextDataObject *copy_data = NULL;
	try {
		std::string copy_text = tab->doc->handle_copy(cut);
		if(!copy_text.empty())
		{
			copy_data = new wxTextDataObject(copy_text);
		}
	}
	catch(const std::bad_alloc &e)
	{
		wxMessageBox(
			"Memory allocation failed while preparing clipboard buffer.",
			"Error", (wxOK | wxICON_ERROR), this);
		return;
	}
	catch(const std::exception &e)
	{
		wxMessageBox(e.what(), "Error", (wxOK | wxICON_ERROR), this);
		return;
	}
	
	if(copy_data != NULL)
	{
		ClipboardGuard cg;
		if(cg)
		{
			wxTheClipboard->SetData(copy_data);
		}
		else{
			delete copy_data;
		}
	}
}

bool REHex::MainWindow::unsaved_confirm()
{
	std::vector<wxString> dirty_files;
	
	size_t num_tabs = notebook->GetPageCount();
	for(size_t i = 0; i < num_tabs; ++i)
	{
		wxWindow *page = notebook->GetPage(i);
		assert(page != NULL);
		
		auto tab = dynamic_cast<REHex::MainWindow::Tab*>(page);
		assert(tab != NULL);
		
		if(tab->doc->is_dirty())
		{
			dirty_files.push_back(tab->doc->get_title());
		}
	}
	
	return unsaved_confirm(dirty_files);
}

bool REHex::MainWindow::unsaved_confirm(const std::vector<wxString> &files)
{
	if(files.size() == 1)
	{
		wxMessageDialog confirm(this, (wxString("The file ") + files[0] + " has unsaved changes.\nClose anyway?"), "Unsaved changes",
			(wxYES | wxNO | wxCENTER));
		
		int response = confirm.ShowModal();
		
		return response == wxID_YES;
	}
	else if(files.size() > 1)
	{
		wxString message = "The following files have unsaved changes, close anyway?\n";
		
		for(auto i = files.begin(); i != files.end(); ++i)
		{
			message.Append('\n');
			message.Append(*i);
		}
		
		wxMessageDialog confirm(this, message, "Unsaved changes",
			(wxYES | wxNO | wxCENTER));
		
		int response = confirm.ShowModal();
		
		return response == wxID_YES;
	}

	return true;
}

void REHex::MainWindow::close_tab(REHex::MainWindow::Tab *tab)
{
	if(tab->doc->is_dirty())
	{
		std::vector<wxString> dirty_titles;
		dirty_titles.push_back(tab->doc->get_title());
		
		if(!unsaved_confirm(dirty_titles))
		{
			/* User didn't really want to close unsaved tabs. */
			return;
		}
	}
	
	notebook->DeletePage(notebook->GetPageIndex(tab));
	
	if(notebook->GetPageCount() == 0)
	{
		ProcessCommand(wxID_NEW);
	}
}

void REHex::MainWindow::close_all_tabs()
{
	if(!unsaved_confirm())
	{
		/* User didn't really want to close unsaved tabs. */
		return;
	}
	
	notebook->DeleteAllPages();
	ProcessCommand(wxID_NEW);
}

void REHex::MainWindow::close_other_tabs(REHex::MainWindow::Tab *tab)
{
	std::vector<wxString> dirty_others;
	
	size_t num_tabs = notebook->GetPageCount();
	for(size_t i = 0; i < num_tabs; ++i)
	{
		wxWindow *page = notebook->GetPage(i);
		assert(page != NULL);
		
		if(page == tab)
		{
			continue;
		}
		
		auto p_tab = dynamic_cast<REHex::MainWindow::Tab*>(page);
		assert(p_tab != NULL);
		
		if(p_tab->doc->is_dirty())
		{
			dirty_others.push_back(p_tab->doc->get_title());
		}
	}
	
	if(!unsaved_confirm(dirty_others))
	{
		/* User didn't really want to close unsaved tabs. */
		return;
	}
	
	for(size_t i = 0; i < notebook->GetPageCount();)
	{
		wxWindow *page = notebook->GetPage(i);
		assert(page != NULL);
		
		if(page == tab)
		{
			++i;
		}
		else{
			notebook->DeletePage(i);
		}
	}
}

BEGIN_EVENT_TABLE(REHex::MainWindow::Tab, wxPanel)
	EVT_SIZE(REHex::MainWindow::Tab::OnSize)
	
	EVT_NOTEBOOK_PAGE_CHANGED(ID_HTOOLS, REHex::MainWindow::Tab::OnHToolChange)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_VTOOLS, REHex::MainWindow::Tab::OnVToolChange)
	
	EVT_SPLITTER_SASH_POS_CHANGING(ID_HSPLITTER, REHex::MainWindow::Tab::OnHSplitterSashPosChanging)
	EVT_SPLITTER_SASH_POS_CHANGING(ID_VSPLITTER, REHex::MainWindow::Tab::OnVSplitterSashPosChanging)
	
	EVT_OFFSETLENGTH(wxID_ANY, REHex::COMMENT_LEFT_CLICK,  REHex::MainWindow::Tab::OnCommentLeftClick)
	EVT_OFFSETLENGTH(wxID_ANY, REHex::COMMENT_RIGHT_CLICK, REHex::MainWindow::Tab::OnCommentRightClick)
	
	EVT_COMMAND(wxID_ANY, REHex::DATA_RIGHT_CLICK, REHex::MainWindow::Tab::OnDataRightClick)
END_EVENT_TABLE()

REHex::MainWindow::Tab::Tab(wxWindow *parent):
	wxPanel(parent)
{
	v_splitter = new wxSplitterWindow(this, ID_VSPLITTER, wxDefaultPosition, wxDefaultSize, (wxSP_3D | wxSP_LIVE_UPDATE));
	v_splitter->SetSashGravity(1.0);
	v_splitter->SetMinimumPaneSize(20);
	
	h_splitter = new wxSplitterWindow(v_splitter, ID_HSPLITTER, wxDefaultPosition, wxDefaultSize, (wxSP_3D | wxSP_LIVE_UPDATE));
	h_splitter->SetSashGravity(1.0);
	h_splitter->SetMinimumPaneSize(20);
	
	doc = new REHex::Document(h_splitter);
	
	doc_ctrl = new REHex::DocumentCtrl(h_splitter, doc);
	
	doc->Bind(EV_COMMENT_MODIFIED,   [this](wxCommandEvent &event) { repopulate_regions(); event.Skip(); });
	doc->Bind(EV_HIGHLIGHTS_CHANGED, [this](wxCommandEvent &event) { doc_ctrl->Refresh(); event.Skip(); });
	
	doc->Bind(DATA_ERASE,     &REHex::MainWindow::Tab::OnDocumentDataErase,     this);
	doc->Bind(DATA_INSERT,    &REHex::MainWindow::Tab::OnDocumentDataInsert,    this);
	doc->Bind(DATA_OVERWRITE, &REHex::MainWindow::Tab::OnDocumentDataOverwrite, this);
	
	doc->Bind(CURSOR_UPDATE, [this](CursorUpdateEvent &event)
	{
		doc_ctrl->set_cursor_position(event.cursor_pos, event.cursor_state);
		event.Skip();
	});
	
	doc_ctrl->Bind(CURSOR_UPDATE, [this](CursorUpdateEvent &event)
	{
		doc->set_cursor_position(event.cursor_pos, event.cursor_state);
		event.Skip();
	});
	
	doc_ctrl->Bind(wxEVT_CHAR, &REHex::MainWindow::Tab::OnDocumentCtrlChar, this);
	
	repopulate_regions();
	
	init_default_doc_view();
	doc_ctrl->set_insert_mode(true);
	
	h_tools = new wxNotebook(h_splitter, ID_HTOOLS, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);
	h_tools->SetFitToCurrentPage(true);
	
	v_tools = new wxNotebook(v_splitter, ID_VTOOLS, wxDefaultPosition, wxDefaultSize, wxNB_RIGHT);
	v_tools->SetFitToCurrentPage(true);
	
	h_splitter->SplitHorizontally(doc_ctrl, h_tools);
	v_splitter->SplitVertically(h_splitter, v_tools);
	
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(v_splitter, 1, wxEXPAND);
	SetSizerAndFit(sizer);
	
	init_default_tools();
	
	htools_adjust_on_idle();
	vtools_adjust_on_idle();
}

REHex::MainWindow::Tab::Tab(wxWindow *parent, const std::string &filename):
	wxPanel(parent)
{
	v_splitter = new wxSplitterWindow(this, ID_VSPLITTER, wxDefaultPosition, wxDefaultSize, (wxSP_3D | wxSP_LIVE_UPDATE));
	v_splitter->SetSashGravity(1.0);
	v_splitter->SetMinimumPaneSize(20);
	
	h_splitter = new wxSplitterWindow(v_splitter, ID_HSPLITTER, wxDefaultPosition, wxDefaultSize, (wxSP_3D | wxSP_LIVE_UPDATE));
	h_splitter->SetSashGravity(1.0);
	h_splitter->SetMinimumPaneSize(20);
	
	doc = new REHex::Document(h_splitter, filename);
	
	doc_ctrl = new REHex::DocumentCtrl(h_splitter, doc);
	
	doc->Bind(EV_COMMENT_MODIFIED,   [this](wxCommandEvent &event) { repopulate_regions(); event.Skip(); });
	doc->Bind(EV_HIGHLIGHTS_CHANGED, [this](wxCommandEvent &event) { doc_ctrl->Refresh(); event.Skip(); });
	
	doc->Bind(DATA_ERASE,     &REHex::MainWindow::Tab::OnDocumentDataErase,     this);
	doc->Bind(DATA_INSERT,    &REHex::MainWindow::Tab::OnDocumentDataInsert,    this);
	doc->Bind(DATA_OVERWRITE, &REHex::MainWindow::Tab::OnDocumentDataOverwrite, this);
	
	doc->Bind(CURSOR_UPDATE, [this](CursorUpdateEvent &event)
	{
		doc_ctrl->set_cursor_position(event.cursor_pos, event.cursor_state);
		event.Skip();
	});
	
	doc_ctrl->Bind(CURSOR_UPDATE, [this](CursorUpdateEvent &event)
	{
		doc->set_cursor_position(event.cursor_pos, event.cursor_state);
		event.Skip();
	});
	
	doc_ctrl->Bind(wxEVT_CHAR, &REHex::MainWindow::Tab::OnDocumentCtrlChar, this);
	
	repopulate_regions();
	
	init_default_doc_view();
	
	h_tools = new wxNotebook(h_splitter, ID_HTOOLS, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);
	h_tools->SetFitToCurrentPage(true);
	
	v_tools = new wxNotebook(v_splitter, ID_VTOOLS, wxDefaultPosition, wxDefaultSize, wxNB_RIGHT);
	v_tools->SetFitToCurrentPage(true);
	
	h_splitter->SplitHorizontally(doc_ctrl, h_tools);
	v_splitter->SplitVertically(h_splitter, v_tools);
	
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(v_splitter, 1, wxEXPAND);
	SetSizerAndFit(sizer);
	
	init_default_tools();
	
	htools_adjust_on_idle();
	vtools_adjust_on_idle();
}

REHex::MainWindow::Tab::~Tab()
{
	for(auto sdi = search_dialogs.begin(); sdi != search_dialogs.end(); ++sdi)
	{
		(*sdi)->Unbind(wxEVT_DESTROY, &REHex::MainWindow::Tab::OnSearchDialogDestroy, this);
	}
}

bool REHex::MainWindow::Tab::tool_active(const std::string &name)
{
	return tools.find(name) != tools.end();
}

void REHex::MainWindow::Tab::tool_create(const std::string &name, bool switch_to, wxConfig *config, bool adjust)
{
	if(tool_active(name))
	{
		return;
	}
	
	const ToolPanelRegistration *tpr = ToolPanelRegistry::by_name(name);
	assert(tpr != NULL);
	
	if(tpr->shape == ToolPanel::TPS_TALL)
	{
		ToolPanel *tool_window = tpr->factory(v_tools, doc, doc_ctrl);
		if(config)
		{
			tool_window->load_state(config);
		}
		
		v_tools->AddPage(tool_window, tpr->label, switch_to);
		
		tools.insert(std::make_pair(name, tool_window));
		
		if(adjust)
		{
			vtools_adjust_on_idle();
		}
	}
	else if(tpr->shape == ToolPanel::TPS_WIDE)
	{
		ToolPanel *tool_window = tpr->factory(h_tools, doc, doc_ctrl);
		if(config)
		{
			tool_window->load_state(config);
		}
		
		h_tools->AddPage(tool_window, tpr->label, switch_to);
		
		tools.insert(std::make_pair(name, tool_window));
		
		if(adjust)
		{
			htools_adjust_on_idle();
		}
	}
}

void REHex::MainWindow::Tab::tool_destroy(const std::string &name)
{
	auto ti = tools.find(name);
	if(ti == tools.end())
	{
		return;
	}
	
	wxWindow *tool_window = ti->second;
	tools.erase(ti);
	
	wxNotebook *notebook = dynamic_cast<wxNotebook*>(tool_window->GetParent());
	assert(notebook != NULL);
	
	int page_idx = notebook->FindPage(tool_window);
	assert(page_idx != wxNOT_FOUND);
	
	notebook->DeletePage(page_idx);
	
	if(notebook == v_tools)
	{
		vtools_adjust();
	}
	else if(notebook == h_tools)
	{
		htools_adjust();
	}
}

void REHex::MainWindow::Tab::search_dialog_register(wxDialog *search_dialog)
{
	search_dialogs.insert(search_dialog);
	search_dialog->Bind(wxEVT_DESTROY, &REHex::MainWindow::Tab::OnSearchDialogDestroy, this);
}

void REHex::MainWindow::Tab::save_view(wxConfig *config)
{
	config->SetPath("/");
	config->Write("theme", wxString(active_palette->get_name()));
	
	config->DeleteGroup("/default-view/");
	config->SetPath("/default-view/");
	
	config->Write("bytes-per-line", doc_ctrl->get_bytes_per_line());
	config->Write("bytes-per-group", doc_ctrl->get_bytes_per_group());
	config->Write("show-offsets", doc_ctrl->get_show_offsets());
	config->Write("show-ascii", doc_ctrl->get_show_ascii());
	config->Write("inline-comments", (int)(inline_comment_mode));
	config->Write("highlight-selection-match", doc_ctrl->get_highlight_selection_match());
	config->Write("offset-display-base", (int)(doc_ctrl->get_offset_display_base()));
	
	/* TODO: Save h_tools state */
	
	for(size_t i = 0; i < v_tools->GetPageCount(); ++i)
	{
		char path[64];
		snprintf(path, sizeof(path), "/default-view/vtools/panels/0/tab/%u/", (unsigned)(i));
		
		config->SetPath(path);
		
		wxWindow *page = v_tools->GetPage(i);
		assert(page != NULL);
		
		ToolPanel *tp = dynamic_cast<ToolPanel*>(page);
		assert(tp != NULL);
		
		config->Write("name", wxString(tp->name()));
		config->Write("selected", (page == v_tools->GetCurrentPage()));
		tp->save_state(config);
	}
}

void REHex::MainWindow::Tab::OnSize(wxSizeEvent &event)
{
	if(h_splitter->IsSplit())
	{
		int hs_sp = h_splitter->GetSashPosition();
		int hs_cp = hsplit_clamp_sash(hs_sp);
		
		if(hs_sp != hs_cp)
		{
			h_splitter->SetSashPosition(hs_cp);
		}
	}
	
	if(v_splitter->IsSplit())
	{
		int vs_sp = v_splitter->GetSashPosition();
		int vs_cp = vsplit_clamp_sash(vs_sp);
		
		if(vs_sp != vs_cp)
		{
			v_splitter->SetSashPosition(vs_cp);
		}
	}
	
	/* Continue propogation of EVT_SIZE event. */
	event.Skip();
}

void REHex::MainWindow::Tab::OnHToolChange(wxNotebookEvent& event)
{
	htools_adjust();
}

void REHex::MainWindow::Tab::OnVToolChange(wxBookCtrlEvent &event)
{
	vtools_adjust();
}

void REHex::MainWindow::Tab::OnHSplitterSashPosChanging(wxSplitterEvent &event)
{
	int pos = event.GetSashPosition();
	int clamp = hsplit_clamp_sash(pos);
	
	if(pos != clamp)
	{
		event.SetSashPosition(clamp);
	}
}

void REHex::MainWindow::Tab::OnVSplitterSashPosChanging(wxSplitterEvent &event)
{
	int pos = event.GetSashPosition();
	int clamp = vsplit_clamp_sash(pos);
	
	if(pos != clamp)
	{
		event.SetSashPosition(clamp);
	}
}

void REHex::MainWindow::Tab::OnSearchDialogDestroy(wxWindowDestroyEvent &event)
{
	search_dialogs.erase((wxDialog*)(event.GetWindow()));
	
	/* Continue propogation. */
	event.Skip();
}

void REHex::MainWindow::Tab::OnDocumentCtrlChar(wxKeyEvent &event)
{
	int key       = event.GetKeyCode();
	int modifiers = event.GetModifiers();
	
	off_t cursor_pos = doc_ctrl->get_cursor_position();
	
	auto selection = doc_ctrl->get_selection();
	off_t selection_off = selection.first;
	off_t selection_length = selection.second;
	
	bool insert_mode = doc_ctrl->get_insert_mode();
	
	Document::CursorState cursor_state = doc_ctrl->get_cursor_state();
	
	if(cursor_state != Document::CSTATE_ASCII && (modifiers == wxMOD_NONE || modifiers == wxMOD_SHIFT) && isasciihex(key))
	{
		unsigned char nibble = REHex::parse_ascii_nibble(key);
		
		if(cursor_state == Document::CSTATE_HEX_MID)
		{
			/* Overwrite least significant nibble of current byte, then move onto
			 * inserting or overwriting at the next byte.
			*/
			
			std::vector<unsigned char> cur_data = doc->read_data(cursor_pos, 1);
			assert(cur_data.size() == 1);
			
			unsigned char old_byte = cur_data[0];
			unsigned char new_byte = (old_byte & 0xF0) | nibble;
			
			doc->overwrite_data(cursor_pos, &new_byte, 1, cursor_pos + 1, Document::CSTATE_HEX, "change data");
		}
		else if(insert_mode)
		{
			/* Inserting a new byte. Initialise the most significant nibble then move
			 * onto overwriting the least significant.
			*/
			
			unsigned char byte = (nibble << 4);
			doc->insert_data(cursor_pos, &byte, 1, cursor_pos, Document::CSTATE_HEX_MID, "change data");
		}
		else{
			/* Overwrite most significant nibble of current byte, then move onto
			 * overwriting the least significant.
			*/
			
			std::vector<unsigned char> cur_data = doc->read_data(cursor_pos, 1);
			
			if(!cur_data.empty())
			{
				unsigned char old_byte = cur_data[0];
				unsigned char new_byte = (old_byte & 0x0F) | (nibble << 4);
				
				doc->overwrite_data(cursor_pos, &new_byte, 1, cursor_pos, Document::CSTATE_HEX_MID, "change data");
			}
		}
		
		doc_ctrl->clear_selection();
		
		return;
	}
	else if(cursor_state == Document::CSTATE_ASCII && (modifiers == wxMOD_NONE || modifiers == wxMOD_SHIFT) && isasciiprint(key))
	{
		unsigned char byte = key;
		
		if(insert_mode)
		{
			doc->insert_data(cursor_pos, &byte, 1, cursor_pos + 1, Document::CSTATE_ASCII, "change data");
		}
		else if(cursor_pos < doc->buffer_length())
		{
			std::vector<unsigned char> cur_data = doc->read_data(cursor_pos, 1);
			assert(cur_data.size() == 1);
			
			doc->overwrite_data(cursor_pos, &byte, 1, cursor_pos + 1, Document::CSTATE_ASCII, "change data");
		}
		
		doc_ctrl->clear_selection();
		
		return;
	}
	else if(modifiers == wxMOD_NONE)
	{
		if(key == WXK_INSERT)
		{
			doc_ctrl->set_insert_mode(!insert_mode);
			return;
		}
		else if(key == WXK_DELETE)
		{
			if(selection_length > 0)
			{
				doc->erase_data(selection_off, selection_length, -1, Document::CSTATE_CURRENT, "delete selection");
			}
			else if(cursor_pos < doc->buffer_length())
			{
				doc->erase_data(cursor_pos, 1, -1, Document::CSTATE_CURRENT, "delete");
			}
			
			return;
		}
		else if(key == WXK_BACK)
		{
			if(selection_length > 0)
			{
				doc->erase_data(selection_off, selection_length, -1, Document::CSTATE_CURRENT, "delete selection");
			}
			else if(cursor_state == Document::CSTATE_HEX_MID)
			{
				/* Backspace while waiting for the second nibble in a byte should erase the current byte
				 * rather than the previous one.
				*/
				doc->erase_data(cursor_pos, 1, -1, Document::CSTATE_CURRENT, "delete");
			}
			else if(cursor_pos > 0)
			{
				doc->erase_data(cursor_pos - 1, 1, -1, Document::CSTATE_CURRENT, "delete");
			}
			
			return;
		}
		else if(key == '/')
		{
			if(cursor_pos < doc->buffer_length())
			{
				EditCommentDialog::run_modal(this, doc, cursor_pos, 0);
			}
			
			return;
		}
	}
	
	event.Skip();
}

void REHex::MainWindow::Tab::OnCommentLeftClick(OffsetLengthEvent &event)
{
	EditCommentDialog::run_modal(this, doc, event.offset, event.length);
}

void REHex::MainWindow::Tab::OnCommentRightClick(OffsetLengthEvent &event)
{
	off_t c_offset = event.offset;
	off_t c_length = event.length;
	
	wxMenu menu;
	
	wxMenuItem *edit_comment = menu.Append(wxID_ANY, "&Edit comment");
	menu.Bind(wxEVT_MENU, [&](wxCommandEvent &event)
	{
		EditCommentDialog::run_modal(this, doc, c_offset, c_length);
	}, edit_comment->GetId(), edit_comment->GetId());
	
	wxMenuItem *delete_comment = menu.Append(wxID_ANY, "&Delete comment");
	menu.Bind(wxEVT_MENU, [&](wxCommandEvent &event)
	{
		doc->erase_comment(c_offset, c_length);
	}, delete_comment->GetId(), delete_comment->GetId());
	
	menu.AppendSeparator();
	
	wxMenuItem *copy_comments = menu.Append(wxID_ANY,  "&Copy comment(s)");
	menu.Bind(wxEVT_MENU, [&](wxCommandEvent &event)
	{
		ClipboardGuard cg;
		if(cg)
		{
			const NestedOffsetLengthMap<Document::Comment> &comments = doc->get_comments();
			
			auto selected_comments = NestedOffsetLengthMap_get_recursive(comments, NestedOffsetLengthMapKey(c_offset, c_length));
			assert(selected_comments.size() > 0);
			
			wxTheClipboard->SetData(new CommentsDataObject(selected_comments, c_offset));
		}
	}, copy_comments->GetId(), copy_comments->GetId());
	
	PopupMenu(&menu);
}

void REHex::MainWindow::Tab::OnDataRightClick(wxCommandEvent &event)
{
	off_t cursor_pos = doc_ctrl->get_cursor_position();
	
	auto selection = doc_ctrl->get_selection();
	off_t selection_off = selection.first;
	off_t selection_length = selection.second;
	
	const NestedOffsetLengthMap<Document::Comment> &comments   = doc->get_comments();
	const NestedOffsetLengthMap<int>               &highlights = doc->get_highlights();
	
	wxMenu menu;
	
	menu.Append(wxID_CUT, "Cu&t");
	menu.Enable(wxID_CUT,  (selection_length > 0));
	
	menu.Append(wxID_COPY,  "&Copy");
	menu.Enable(wxID_COPY, (selection_length > 0));
	
	menu.Append(wxID_PASTE, "&Paste");
	
	menu.AppendSeparator();
	
	wxMenuItem *offset_copy_hex = menu.Append(wxID_ANY, "Copy offset (in hexadecimal)");
	menu.Bind(wxEVT_MENU, [cursor_pos](wxCommandEvent &event)
	{
		ClipboardGuard cg;
		if(cg)
		{
			char offset_str[24];
			snprintf(offset_str, sizeof(offset_str), "0x%llX", (long long unsigned)(cursor_pos));
			
			wxTheClipboard->SetData(new wxTextDataObject(offset_str));
		}
	}, offset_copy_hex->GetId(), offset_copy_hex->GetId());
	
	wxMenuItem *offset_copy_dec = menu.Append(wxID_ANY, "Copy offset (in decimal)");
	menu.Bind(wxEVT_MENU, [cursor_pos](wxCommandEvent &event)
	{
		ClipboardGuard cg;
		if(cg)
		{
			char offset_str[24];
			snprintf(offset_str, sizeof(offset_str), "%llu", (long long unsigned)(cursor_pos));
			
			wxTheClipboard->SetData(new wxTextDataObject(offset_str));
		}
	}, offset_copy_dec->GetId(), offset_copy_dec->GetId());
	
	menu.AppendSeparator();
	
	auto comments_at_cur = NestedOffsetLengthMap_get_all(comments, cursor_pos);
	for(auto i = comments_at_cur.begin(); i != comments_at_cur.end(); ++i)
	{
		auto ci = *i;
		
		wxString text = ci->second.menu_preview();
		wxMenuItem *itm = menu.Append(wxID_ANY, wxString("Edit \"") + text + "\"...");
		
		menu.Bind(wxEVT_MENU, [this, ci](wxCommandEvent &event)
		{
			EditCommentDialog::run_modal(this, doc, ci->first.offset, ci->first.length);
		}, itm->GetId(), itm->GetId());
	}
	
	if(comments.find(NestedOffsetLengthMapKey(cursor_pos, 0)) == comments.end()
		&& cursor_pos < doc->buffer_length())
	{
		wxMenuItem *itm = menu.Append(wxID_ANY, "Insert comment here...");
		
		menu.Bind(wxEVT_MENU, [this, cursor_pos](wxCommandEvent &event)
		{
			EditCommentDialog::run_modal(this, doc, cursor_pos, 0);
		}, itm->GetId(), itm->GetId());
	}
	
	if(selection_length > 0
		&& comments.find(NestedOffsetLengthMapKey(selection_off, selection_length)) == comments.end()
		&& NestedOffsetLengthMap_can_set(comments, selection_off, selection_length))
	{
		char menu_label[64];
		snprintf(menu_label, sizeof(menu_label), "Set comment on %" PRId64 " bytes...", (int64_t)(selection_length));
		wxMenuItem *itm =  menu.Append(wxID_ANY, menu_label);
		
		menu.Bind(wxEVT_MENU, [&](wxCommandEvent &event)
		{
			EditCommentDialog::run_modal(this, doc, selection_off, selection_length);
		}, itm->GetId(), itm->GetId());
	}
	
	menu.AppendSeparator();
	
	/* We need to maintain bitmap instances for lifespan of menu. */
	std::list<wxBitmap> bitmaps;
	
	off_t highlight_off;
	off_t highlight_length = 0;
	
	auto highlight_at_cur = NestedOffsetLengthMap_get(highlights, cursor_pos);
	
	if(selection_length > 0)
	{
		highlight_off    = selection_off;
		highlight_length = selection_length;
	}
	else if(highlight_at_cur != highlights.end())
	{
		highlight_off    = highlight_at_cur->first.offset;
		highlight_length = highlight_at_cur->first.length;
	}
	else if(cursor_pos < doc->buffer_length())
	{
		highlight_off    = cursor_pos;
		highlight_length = 1;
	}
	
	if(highlight_length > 0 && NestedOffsetLengthMap_can_set(highlights, highlight_off, highlight_length))
	{
		wxMenu *hlmenu = new wxMenu();
		
		for(int i = 0; i < Palette::NUM_HIGHLIGHT_COLOURS; ++i)
		{
			wxMenuItem *itm = new wxMenuItem(hlmenu, wxID_ANY, " ");
			
			wxColour bg_colour = active_palette->get_highlight_bg(i);
			
			/* TODO: Get appropriate size for menu bitmap.
			 * TODO: Draw a character in image using foreground colour.
			*/
			wxImage img(16, 16);
			img.SetRGB(wxRect(0, 0, img.GetWidth(), img.GetHeight()),
				bg_colour.Red(), bg_colour.Green(), bg_colour.Blue());
			
			bitmaps.emplace_back(img);
			itm->SetBitmap(bitmaps.back());
			
			hlmenu->Append(itm);
			
			/* On Windows, event bindings on a submenu don't work.
			 * On OS X, event bindings on a parent menu don't work.
			 * On GTK, both work.
			*/
			#ifdef _WIN32
			menu.Bind(wxEVT_MENU, [this, highlight_off, highlight_length, i](wxCommandEvent &event)
			#else
			hlmenu->Bind(wxEVT_MENU, [this, highlight_off, highlight_length, i](wxCommandEvent &event)
			#endif
			{
				int colour = i;
				doc->set_highlight(highlight_off, highlight_length, colour);
			}, itm->GetId(), itm->GetId());
		}
		
		menu.AppendSubMenu(hlmenu, "Set Highlight");
	}
	
	if(highlight_at_cur != highlights.end())
	{
		wxMenuItem *itm = menu.Append(wxID_ANY, "Remove Highlight");
		
		NestedOffsetLengthMapKey key = highlight_at_cur->first;
		
		menu.Bind(wxEVT_MENU, [this, key](wxCommandEvent &event)
		{
			doc->erase_highlight(key.offset, key.length);
		}, itm->GetId(), itm->GetId());
	}
	
	if(selection_length > 0)
	{
		menu.AppendSeparator();
		wxMenuItem *itm = menu.Append(wxID_ANY, "Compare...");
		
		menu.Bind(wxEVT_MENU, [this, selection_off, selection_length](wxCommandEvent &event)
		{
			static DiffWindow *diff = NULL;
			if(diff == NULL)
			{
				diff = new DiffWindow();
				diff->Show(true);
			}
			
			diff->add_range(DiffWindow::Range(doc, selection_off, selection_length));
		}, itm->GetId(), itm->GetId());
	}
	
	PopupMenu(&menu);
}

void REHex::MainWindow::Tab::OnDocumentDataErase(OffsetLengthEvent &event)
{
	repopulate_regions();
	event.Skip();
}

void REHex::MainWindow::Tab::OnDocumentDataInsert(OffsetLengthEvent &event)
{
	repopulate_regions();
	event.Skip();
}

void REHex::MainWindow::Tab::OnDocumentDataOverwrite(OffsetLengthEvent &event)
{
	doc_ctrl->Refresh();
	event.Skip();
}

int REHex::MainWindow::Tab::hsplit_clamp_sash(int sash_position)
{
	/* Prevent the user resizing a tool panel beyond its min/max size.
	 * NOTE: Minimuim size is clamped >= 0 to prevent the size shrinking past the wxNotebook
	 * control itself, else weird rendering/input glitches happen.
	*/
	
	wxWindow *ht_current_page = h_tools->GetCurrentPage();
	if(ht_current_page == NULL)
	{
		/* No active page to reference. */
		return sash_position;
	}
	
	int htp_mh = std::max(ht_current_page->GetMinSize().GetHeight(), 0);
	int htp_Mh = ht_current_page->GetMaxSize().GetHeight();
	
	int hs_ch = h_splitter->GetClientSize().GetHeight();
	int hs_ss = h_splitter->GetSashSize();
	
	/* Size oherhead added by h_tools wxNotebook. */
	int extra_h = h_tools->GetSize().GetHeight() - ht_current_page->GetSize().GetHeight();
	
	int sash_max = hs_ch - (htp_mh + extra_h + hs_ss);
	if(sash_position > sash_max)
	{
		return sash_max;
	}
	
	if(htp_Mh > 0)
	{
		int sash_min = hs_ch - (htp_Mh + extra_h + hs_ss);
		if(sash_position < sash_min)
		{
			return sash_min;
		}
	}
	
	return sash_position;
}

int REHex::MainWindow::Tab::vsplit_clamp_sash(int sash_position)
{
	/* Prevent the user resizing a tool panel beyond its min/max size.
	 * NOTE: Minimuim size is clamped >= 0 to prevent the size shrinking past the wxNotebook
	 * control itself, else weird rendering/input glitches happen.
	*/
	
	wxWindow *vt_current_page = v_tools->GetCurrentPage();
	if(vt_current_page == NULL)
	{
		/* No active page to reference. */
		return sash_position;
	}
	
	int vtp_mw = std::max(vt_current_page->GetMinSize().GetWidth(), 0);
	int vtp_Mw = vt_current_page->GetMaxSize().GetWidth();
	
	int vs_cw = v_splitter->GetClientSize().GetWidth();
	int vs_ss = v_splitter->GetSashSize();
	
	/* Size overhead added by v_tools wxNotebook. */
	int extra_w = v_tools->GetSize().GetWidth() - vt_current_page->GetSize().GetWidth();
	
	int sash_max = vs_cw - (vtp_mw + extra_w + vs_ss);
	if(sash_position > sash_max)
	{
		return sash_max;
	}
	
	if(vtp_Mw > 0)
	{
		int sash_min = vs_cw - (vtp_Mw + extra_w + vs_ss);
		if(sash_position < sash_min)
		{
			return sash_min;
		}
	}
	
	return sash_position;
}

void REHex::MainWindow::Tab::vtools_adjust()
{
	wxWindow *vt_current_page = v_tools->GetCurrentPage();
	
	if(vt_current_page == NULL || !vt_current_page->IsShown())
	{
		/* Vertical tool pane has no pages, or the page is hidden. Hide it. */
		if(v_splitter->IsSplit())
		{
			v_splitter->Unsplit();
		}
	}
	else{
		if(!v_splitter->IsSplit())
		{
			v_splitter->SplitVertically(h_splitter, v_tools);
		}
		
		int vtp_bw = std::max(vt_current_page->GetBestSize().GetWidth(), 0);
		
		/* Size overhead added by v_tools wxNotebook. */
		int extra_w = v_tools->GetSize().GetWidth() - vt_current_page->GetSize().GetWidth();
		
		/* Set the current position of the splitter to display the best size of the current
		 * page and overhead.
		*/
		int vs_cw = v_splitter->GetClientSize().GetWidth();
		v_splitter->SetSashPosition(vs_cw - (vtp_bw + extra_w + v_splitter->GetSashSize()));
	}
}

void REHex::MainWindow::Tab::htools_adjust()
{
	wxWindow *ht_current_page = h_tools->GetCurrentPage();
	
	if(ht_current_page == NULL || !ht_current_page->IsShown())
	{
		/* Horizontal tool pane has no pages, or the page is hidden. Hide it. */
		if(h_splitter->IsSplit())
		{
			h_splitter->Unsplit();
		}
	}
	else{
		if(!h_splitter->IsSplit())
		{
			h_splitter->SplitHorizontally(doc_ctrl, h_tools);
		}
		
		int htp_bh = std::max(ht_current_page->GetBestSize().GetHeight(), 0);
		
		/* Size overhead added by h_tools wxNotebook. */
		int extra_h = h_tools->GetSize().GetHeight() - ht_current_page->GetSize().GetHeight();
		
		/* Set the sash position to display the tool page's best size. */
		int hs_ch = h_splitter->GetClientSize().GetHeight();
		h_splitter->SetSashPosition(hs_ch - (htp_bh + extra_h + h_splitter->GetSashSize()));
	}
}

/* The size of a wxNotebook page doesn't seem to be set correctly during
 * initialisation (or immediately after adding a page), so we can't use it to
 * determine how much size overhead the wxNotebook adds at that point. Instead
 * we defer setting of the tool pane sizes until the first idle tick, by which
 * point the sizes seem to have been set up properly (on GTK anyway).
*/

void REHex::MainWindow::Tab::vtools_adjust_on_idle()
{
	Bind(wxEVT_IDLE, &REHex::MainWindow::Tab::vtools_adjust_now_idle, this);
}

void REHex::MainWindow::Tab::vtools_adjust_now_idle(wxIdleEvent &event)
{
	Unbind(wxEVT_IDLE, &REHex::MainWindow::Tab::vtools_adjust_now_idle, this);
	event.Skip();
	
	vtools_adjust();
}

void REHex::MainWindow::Tab::htools_adjust_on_idle()
{
	Bind(wxEVT_IDLE, &REHex::MainWindow::Tab::htools_adjust_now_idle, this);
}

void REHex::MainWindow::Tab::htools_adjust_now_idle(wxIdleEvent &event)
{
	Unbind(wxEVT_IDLE, &REHex::MainWindow::Tab::htools_adjust_now_idle, this);
	event.Skip();
	
	htools_adjust();
}

void REHex::MainWindow::Tab::init_default_doc_view()
{
	wxConfig *config = wxGetApp().config;
	config->SetPath("/default-view/");
	
	doc_ctrl->set_bytes_per_line(             config->Read    ("bytes-per-line",             doc_ctrl->get_bytes_per_line()));
	doc_ctrl->set_bytes_per_group(            config->Read    ("bytes-per-group",            doc_ctrl->get_bytes_per_group()));
	doc_ctrl->set_show_offsets(               config->ReadBool("show-offsets",               doc_ctrl->get_show_offsets()));
	doc_ctrl->set_show_ascii(                 config->ReadBool("show-ascii",                 doc_ctrl->get_show_ascii()));
	doc_ctrl->set_highlight_selection_match(  config->ReadBool("highlight-selection-match",  doc_ctrl->get_highlight_selection_match()));
	
	int inline_comments = config->Read("inline-comments", (int)(inline_comment_mode));
	if(inline_comments >= 0 && inline_comments <= ICM_MAX)
	{
		inline_comment_mode = (InlineCommentMode)(inline_comments);
		repopulate_regions();
	}
	
	int offset_display_base = config->Read("offset-display-base", (int)(doc_ctrl->get_offset_display_base()));
	if(offset_display_base >= OFFSET_BASE_MIN && offset_display_base <= OFFSET_BASE_MAX)
	{
		doc_ctrl->set_offset_display_base((OffsetBase)(offset_display_base));
	}
}

void REHex::MainWindow::Tab::init_default_tools()
{
	wxConfig *config = wxGetApp().config;
	
	/* TODO: Load h_tools state. */
	
	for(unsigned int i = 0;; ++i)
	{
		char base_p[64];
		snprintf(base_p, sizeof(base_p), "/default-view/vtools/panels/0/tab/%u/", i);
		
		if(config->HasGroup(base_p))
		{
			config->SetPath(base_p);
			
			std::string name = config->Read    ("name", "").ToStdString();
			bool selected    = config->ReadBool("selected", false);
			
			if(ToolPanelRegistry::by_name(name) != NULL)
			{
				tool_create(name, selected, config, false);
			}
			else{
				/* TODO: Some kind of warning? */
			}
		}
		else{
			break;
		}
	}
}

void REHex::MainWindow::Tab::repopulate_regions()
{
	auto comments = doc->get_comments();
	
	bool nest = (inline_comment_mode == ICM_SHORT_INDENT || inline_comment_mode == ICM_FULL_INDENT);
	bool truncate = (inline_comment_mode == ICM_SHORT || inline_comment_mode == ICM_SHORT_INDENT);
	
	/* Construct a list of interlaced comment/data regions. */
	
	auto offset_base = comments.begin();
	off_t next_data = 0, remain_data = doc->buffer_length();
	
	std::list<DocumentCtrl::Region*> regions;
	
	while(remain_data > 0)
	{
		off_t dr_length = remain_data;
		
		assert(offset_base == comments.end() || offset_base->first.offset >= next_data);
		
		/* We process any comments at the same offset from largest to smallest, ensuring
		 * smaller comments are parented to the next-larger one at the same offset.
		 *
		 * This could be optimised by changing the order of keys in the comments map, but
		 * that'll probably break something...
		*/
		
		if(offset_base != comments.end() && offset_base->first.offset == next_data)
		{
			auto next_offset = offset_base;
			while(next_offset != comments.end() && next_offset->first.offset == offset_base->first.offset)
			{
				++next_offset;
			}
			
			auto c = next_offset;
			do {
				--c;
				
				regions.push_back(new DocumentCtrl::CommentRegion(c->first.offset, c->first.length, *(c->second.text), nest, truncate));
				
				if(nest && c->first.length > 0)
				{
					assert(c->first.length <= dr_length);
					dr_length = c->first.length;
				}
			} while(c != offset_base);
			
			offset_base = next_offset;
		}
		
		if(offset_base != comments.end() && dr_length > (offset_base->first.offset - next_data))
		{
			dr_length = offset_base->first.offset - next_data;
		}
		
		regions.push_back(new DocumentCtrl::DataRegionDocHighlight(next_data, dr_length, *doc));
		
		next_data   += dr_length;
		remain_data -= dr_length;
	}
	
	if(regions.empty())
	{
		assert(doc->buffer_length() == 0);
		
		/* Empty buffers need a data region too! */
		regions.push_back(new DocumentCtrl::DataRegionDocHighlight(0, 0, *doc));
	}
	
	doc_ctrl->replace_all_regions(regions);
}

REHex::MainWindow::DropTarget::DropTarget(MainWindow *window):
	window(window) {}

REHex::MainWindow::DropTarget::~DropTarget() {}

bool REHex::MainWindow::DropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	for(size_t i = 0; i < filenames.GetCount(); ++i)
	{
		window->open_file(filenames[i].ToStdString());
	}
	
	return true;
}
