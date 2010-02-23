/*
PowerShellFar module for Far Manager
Copyright (c) 2006 Roman Kuzmin
*/

using System;
using FarNet;
using Microsoft.Win32;

namespace PowerShellFar
{
	/// <summary>
	/// PowerShellFar settings. Exposed as <c>$Psf.Settings</c>
	/// </summary>
	/// <remarks>
	/// Properties <see cref="StartupCode"/> and <see cref="StartupEdit"/> are stored in the registry
	/// and can be changed in the module configuration dialog.
	/// <para>
	/// Other properties are session preferences and normally set in the profile.
	/// </para>
	/// <example>Profile-.ps1</example>
	/// </remarks>
	public sealed class Settings
	{
		/// <summary>
		/// Restores permanent settings.
		/// </summary>
		internal Settings()
		{
			using (RegistryKey key = Entry.Instance.Manager.OpenSubKey(null, false))
			{
				_StartupCode = key.GetValue("StartupCode", string.Empty).ToString();
				_StartupEdit = key.GetValue("StartupEdit", string.Empty).ToString();
			}
		}

		/// <summary>
		/// Saves permanent settings.
		/// </summary>
		public void Save()
		{
			using (RegistryKey key = Entry.Instance.Manager.OpenSubKey(null, true))
			{
				key.SetValue("StartupCode", _StartupCode);
				key.SetValue("StartupEdit", _StartupEdit);
			}
		}

		/// <summary>
		/// Sets intelli-list menu.
		/// </summary>
		public void Intelli(IListMenu menu)
		{
			if (menu == null) throw new ArgumentNullException("menu");
			menu.AutoSelect = _IntelliAutoSelect;
			menu.MaxHeight = _IntelliMaxHeight;
			menu.NoShadow = _IntelliNoShadow;
		}

		/// <summary>
		/// Sets list menu.
		/// </summary>
		public void ListMenu(IListMenu menu)
		{
			if (menu == null) throw new ArgumentNullException("menu");
			menu.FilterOptions = _ListMenuFilterOptions;
			menu.FilterKey = _ListMenuFilterKey;
			menu.ScreenMargin = _ListMenuScreenMargin;
			menu.UsualMargins = _ListMenuUsualMargins;
		}

		string _StartupCode;
		/// <summary>
		/// See .hlf
		/// </summary>
		public string StartupCode
		{
			get { return _StartupCode; }
			set { _StartupCode = value; }
		}

		string _StartupEdit;
		/// <summary>
		/// See .hlf
		/// </summary>
		public string StartupEdit
		{
			get { return _StartupEdit; }
			set { _StartupEdit = value; }
		}

		bool _IntelliAutoSelect = true;
		/// <summary>
		/// <see cref="IListMenu.AutoSelect"/> for intellisense menus.
		/// </summary>
		public bool IntelliAutoSelect
		{
			get { return _IntelliAutoSelect; }
			set { _IntelliAutoSelect = value; }
		}

		int _IntelliMaxHeight = -1;
		/// <summary>
		/// <see cref="IAnyMenu.MaxHeight"/> for intellisense menus.
		/// </summary>
		public int IntelliMaxHeight
		{
			get { return _IntelliMaxHeight; }
			set { _IntelliMaxHeight = value; }
		}

		bool _IntelliNoShadow;
		/// <summary>
		/// <see cref="IListMenu.NoShadow"/> for intellisense menus.
		/// </summary>
		public bool IntelliNoShadow
		{
			get { return _IntelliNoShadow; }
			set { _IntelliNoShadow = value; }
		}

		PatternOptions _ListMenuFilterOptions = PatternOptions.Regex;
		/// <summary>
		/// <see cref="IListMenu.FilterOptions"/> for list menus.
		/// </summary>
		public PatternOptions ListMenuFilterOptions
		{
			get { return _ListMenuFilterOptions; }
			set { _ListMenuFilterOptions = value; }
		}

		int _ListMenuFilterKey = (KeyMode.Ctrl | KeyCode.Down);
		/// <summary>
		/// <see cref="IListMenu.FilterKey"/> for list menus.
		/// </summary>
		public int ListMenuFilterKey
		{
			get { return _ListMenuFilterKey; }
			set { _ListMenuFilterKey = value; }
		}

		int _ListMenuScreenMargin = 2;
		/// <summary>
		/// <see cref="IListMenu.ScreenMargin"/> for list menus.
		/// </summary>
		public int ListMenuScreenMargin
		{
			get { return _ListMenuScreenMargin; }
			set { _ListMenuScreenMargin = value; }
		}

		bool _ListMenuUsualMargins = true;
		/// <summary>
		/// <see cref="IListMenu.UsualMargins"/> for list menus.
		/// </summary>
		public bool ListMenuUsualMargins
		{
			get { return _ListMenuUsualMargins; }
			set { _ListMenuUsualMargins = value; }
		}

		int _MaximumHistoryCount = 400;
		/// <summary>
		/// The maximum number of history commands kept in the registry. In fact, 10% more is allowed.
		/// </summary>
		public int MaximumHistoryCount
		{
			get { return _MaximumHistoryCount; }
			set { _MaximumHistoryCount = value; }
		}
		
		int _MaximumPanelColumnCount = 8;
		/// <summary>
		/// The maximum number of columns allowed in free format panels.
		/// </summary>
		public int MaximumPanelColumnCount
		{
			get { return _MaximumPanelColumnCount; }
			set
			{
				if (value < 3) throw new ArgumentException(Res.MaximumPanelColumnCount);
				if (value > FarColumn.DefaultColumnKinds.Count) throw new ArgumentException(Res.MaximumPanelColumnCount);
				_MaximumPanelColumnCount = value;
			}
		}
		
		string _ExternalViewerFileName = string.Empty;
		/// <summary>
		/// Gets or sets the external viewer application path.
		/// </summary>
		/// <remarks>
		/// By default it is empty and external Far viewer is used.
		/// <example>
		/// See <see cref="ExternalViewerArguments"/>
		/// </example>
		/// </remarks>
		public string ExternalViewerFileName
		{
			get { return _ExternalViewerFileName; }
			set
			{
				if (value == null) throw new ArgumentNullException("value");
				_ExternalViewerFileName = value;
			}
		}

		string _ExternalViewerArguments = string.Empty;
		/// <summary>
		/// Gets or sets the command line arguments for the external viewer.
		/// </summary>
		/// <remarks>
		/// It is used together with <see cref="ExternalViewerFileName"/>.
		/// Use "{0}" where a file path should be inserted.
		/// <example>
		/// <code>
		/// $Psf.Settings.ExternalViewerFileName = "$env:FARHOME\Far.exe"
		/// $Psf.Settings.ExternalViewerArguments = '/m /p /v "{0}"'
		/// </code>
		/// </example>
		/// </remarks>
		public string ExternalViewerArguments
		{
			get { return _ExternalViewerArguments; }
			set
			{
				if (value == null) throw new ArgumentNullException("value");
				_ExternalViewerArguments = value;
			}
		}

		/// <summary>
		/// Test mode for internal use only.
		/// </summary>
		public long Test { get; set; }
	}
}
