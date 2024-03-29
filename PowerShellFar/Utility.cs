
/*
PowerShellFar module for Far Manager
Copyright (c) 2006-2015 Roman Kuzmin
*/

using System;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Management.Automation;
using System.Management.Automation.Provider;
using System.Management.Automation.Runspaces;
using System.Runtime.Serialization;
using System.Security.Permissions;
using System.Text.RegularExpressions;
using FarNet;
using FarNet.Forms;

namespace PowerShellFar
{
	/// <summary>
	/// For internal use and testing.
	/// </summary>
	public static class Zoo
	{
		///
		public static void Initialize(InitialSessionState state)
		{
			Commands.BaseCmdlet.AddCmdlets(state);
		}
		///
		public static Meta[] TablePanelSetupColumns(object[] columns)
		{
			return Format.SetupColumns(columns);
		}
		[EnvironmentPermissionAttribute(SecurityAction.LinkDemand, Unrestricted = true)]
		internal static Process StartExternalViewer(string fileName)
		{
			string externalViewerFileName = Settings.Default.ExternalViewerFileName;
			string externalViewerArguments;

			// try the user defined viewer
			if (!string.IsNullOrEmpty(externalViewerFileName))
			{
				externalViewerArguments = string.Format(null, Settings.Default.ExternalViewerArguments, fileName);
				try
				{
					return My.ProcessEx.Start(externalViewerFileName, externalViewerArguments);
				}
				catch (Win32Exception)
				{
					Far.Api.Message(
						"Cannot start the external viewer, default viewer will be used.\nYour settings:\nExternalViewerFileName: " + externalViewerFileName + "\nExternalViewerArguments: " + Settings.Default.ExternalViewerArguments,
						Res.Me, MessageOptions.LeftAligned | MessageOptions.Warning);
				}
			}

			// use default external viewer
			externalViewerFileName = Process.GetCurrentProcess().MainModule.FileName;
			externalViewerArguments = "/w- /ro /m /p /v \"" + fileName + "\"";
			return My.ProcessEx.Start(externalViewerFileName, externalViewerArguments);
		}
		#region Transcript
		const string TextTranscriptFileExistsNoClobber = "File {0} already exists and {1} was specified.";
		const string TextTranscriptFileReadOnly = "Transcription file is read only.";
		const string TextTranscriptFileMissing = "Transcription file is missing.";
		const string TextTranscriptInProgress = "Transcription has already been started. Use the Stop-Transcript command to stop transcription.";
		const string TranscriptNotInProgress = "Transcription has not been started. Use the Start-Transcript command to start transcription.";
		const string TextTranscriptStarted = "Transcript started, output file is {0}";
		const string TextTranscriptStopped = "Transcript stopped, output file is {0}";
		///
		[EnvironmentPermissionAttribute(SecurityAction.LinkDemand, Unrestricted = true)]
		public static void ShowTranscript(bool internalViewer)
		{
			if (TranscriptOutputWriter.LastFileName == null)
				throw new InvalidOperationException(TranscriptNotInProgress);

			if (!File.Exists(TranscriptOutputWriter.LastFileName))
				throw new InvalidOperationException(TextTranscriptFileMissing);

			if (internalViewer)
			{
				var viewer = Far.Api.CreateViewer();
				viewer.Title = Path.GetFileName(TranscriptOutputWriter.LastFileName);
				viewer.FileName = TranscriptOutputWriter.LastFileName;
				viewer.CodePage = 1200;
				viewer.Open();
			}
			else
			{
				StartExternalViewer(TranscriptOutputWriter.LastFileName);
			}
		}
		///
		public static string StopTranscript(bool force)
		{
			if (A.Psf.Transcript == null)
			{
				if (force)
					return null;

				throw new InvalidOperationException(TranscriptNotInProgress);
			}

			A.Psf.Transcript.Close();
			A.Psf.Transcript = null;

			return string.Format(null, TextTranscriptStopped, TranscriptOutputWriter.LastFileName);
		}
		///
		public static string StartTranscript(string path, bool append, bool force, bool noClobber)
		{
			if (A.Psf.Transcript != null)
				throw new InvalidOperationException(TextTranscriptInProgress);

			if (string.IsNullOrEmpty(path))
			{
				path = Path.Combine(
					Environment.GetFolderPath(Environment.SpecialFolder.Personal),
					string.Format(null, "PowerShell_transcript.{0:yyyyMMddHHmmss}.txt", DateTime.Now));
			}

			if (File.Exists(path))
			{
				if (noClobber && !append)
					throw new InvalidOperationException(string.Format(null, TextTranscriptFileExistsNoClobber, path, "NoClobber"));

				var fileInfo = new FileInfo(path);
				if ((fileInfo.Attributes & FileAttributes.ReadOnly) == FileAttributes.ReadOnly)
				{
					if (!force)
						throw new InvalidOperationException(TextTranscriptFileReadOnly);

					fileInfo.Attributes &= ~FileAttributes.ReadOnly;
				}
			}

			A.Psf.Transcript = new TranscriptOutputWriter(path, append);

			return string.Format(TextTranscriptStarted, path);
		}
		#endregion
	}

	[Serializable]
	class PowerShellFarException : ModuleException
	{
		public PowerShellFarException() { }
		public PowerShellFarException(string message) : base(message) { }
		public PowerShellFarException(string message, Exception innerException) : base(message, innerException) { }
		protected PowerShellFarException(SerializationInfo info, StreamingContext context) : base(info, context) { }
	}

	/// <summary>
	/// Parameters. Use them to avoid typos.
	/// </summary>
	static class Prm
	{
		public const string
			Confirm = "Confirm",
			ErrorAction = "ErrorAction",
			Force = "Force",
			Recurse = "Recurse";
	}

	/// <summary>
	/// Helper methods.
	/// </summary>
	static class Kit
	{
		/// <summary>
		/// Converts with culture.
		/// </summary>
		public static string ToString<T>(T value) where T : IConvertible //! IConvertible is not CLS-compliant
		{
			return value.ToString(CultureInfo.CurrentCulture);
		}
		/// <summary>
		/// Converts with culture.
		/// </summary>
		public static string ToString(DateTime value, string format)
		{
			return value.ToString(format, CultureInfo.CurrentCulture);
		}
		// Compares strings OrdinalIgnoreCase.
		public static bool Equals(string strA, string strB)
		{
			return string.Equals(strA, strB, StringComparison.OrdinalIgnoreCase);
		}
		// Escapes a literal string to be used as a wildcard.
		//! It is a workaround:
		// 1) Rename-Item has no -LiteralPath --> we have to escape wildcards (anyway it fails e.g. "name`$][").
		// 2) BUG in [Management.Automation.WildcardPattern]::Escape(): e.g. `` is KO ==>.
		// '``' -like [Management.Automation.WildcardPattern]::Escape('``') ==> False
		public static string EscapeWildcard(string literal)
		{
			if (_reEscapeWildcard == null)
				_reEscapeWildcard = new Regex(@"([`\[\]\*\?])");
			return _reEscapeWildcard.Replace(literal, "`$1");
		}
		static Regex _reEscapeWildcard;
		//?? _090901_055134 Check in V2 (bad for viewer and notepad)
		/// <summary>
		/// Formats a position message.
		/// </summary>
		public static string PositionMessage(string message)
		{
			return message.Trim().Replace("\n", "\r\n");
		}
	}

	static class Coloring
	{
		public static void ColorEditAsConsole(object sender, ColoringEventArgs e)
		{
			// normal text
			e.Background1 = ConsoleColor.Black;
			e.Foreground1 = ConsoleColor.Gray;
			// selected text
			e.Background2 = ConsoleColor.White;
			e.Foreground2 = ConsoleColor.DarkGray;
			// unchanged text
			e.Background3 = ConsoleColor.Black;
			e.Foreground3 = ConsoleColor.Gray;
			// combo
			e.Background4 = ConsoleColor.Black;
			e.Foreground4 = ConsoleColor.Gray;
		}
		public static void ColorTextAsConsole(object sender, ColoringEventArgs e)
		{
			// normal text
			e.Background1 = ConsoleColor.Black;
			e.Foreground1 = ConsoleColor.Gray;
		}
	}

	/// <summary>
	/// Extra <see cref="PathInfo"/>.
	/// </summary>
	class PathInfoEx
	{
		readonly PathInfo _PathInfo;
		string _Path;
		///
		public PathInfoEx(string path)
		{
			var core = A.Psf.Engine.SessionState.Path;
			if (string.IsNullOrEmpty(path) || path == ".")
				_PathInfo = core.CurrentLocation;
			else
				// 3 times faster than push/set/pop location; NB: it is slow anyway
				_PathInfo = core.GetResolvedPSPathFromPSPath(Kit.EscapeWildcard(path))[0];
		}
		internal PathInfoEx(PathInfo pathInfo)
		{
			_PathInfo = pathInfo;
		}
		/// <summary>
		/// Gets the friendly path.
		/// </summary>
		public string Path
		{
			get //_110318_140817
			{
				if (_Path == null)
				{
					_Path = _PathInfo.ProviderPath;
					if (!_Path.StartsWith("\\\\", StringComparison.Ordinal))
					{
						_Path = _PathInfo.Path;
						if ((_Path.Length == 0 || _Path == "\\") && _PathInfo.Drive != null)
							_Path = _PathInfo.Drive.Name + ":\\";
					}
				}
				return _Path;
			}
		}
		/// <summary>
		/// Gets the provider info.
		/// </summary>
		public ProviderInfo Provider
		{
			get { return _PathInfo.Provider; }
		}
		/// <summary>
		/// Gets the drive name or null.
		/// </summary>
		internal string DriveName //! 110227 PathInfo.Drive can be null even if a drive exists
		{
			get
			{
				return _PathInfo.Drive == null ? null : _PathInfo.Drive.Name;
			}
		}
	}

	class DataLookup
	{
		string[] _namePairs;
		public DataLookup(string[] namePairs)
		{
			_namePairs = namePairs;
		}
		public void Invoke(object sender, OpenFileEventArgs e)
		{
			// lookup data panel (should be checked, user could use another)
			DataPanel dp = sender as DataPanel;
			if (dp == null)
				throw new InvalidOperationException("Event sender is not a data panel object.");

			// destination row (should be valid, checked on creation by us)
			DataRow drSet = (DataRow)((MemberPanel)dp.Parent).Value.BaseObject;

			// the source row
			DataRow drGet = (DataRow)e.File.Data;

			// copy data using name pairs
			for (int i = 0; i < _namePairs.Length; i += 2)
				drSet[_namePairs[i]] = drGet[_namePairs[i + 1]];
		}
	}

	/// <summary>
	/// User actions.
	/// </summary>
	enum UserAction
	{
		/// <summary>None.</summary>
		None,
		/// <summary>Enter is pressed.</summary>
		Enter
	}

	/// <summary>
	/// Standard message box button set.
	/// </summary>
	public enum ButtonSet
	{
		///
		Ok,
		///
		OkCancel,
		///
		AbortRetryIgnore,
		///
		YesNo,
		///
		YesNoCancel,
		///
		RetryCancel
	}
}

namespace My
{
	static class FileEx
	{
		public static void DeleteIgnoreError(string fileName)
		{
			try
			{
				File.Delete(fileName);
			}
			// in use by another process, often opened by a user, it's fine
			catch (IOException) { }
			// virus scanner, indexing service, it's bad but what can we do?
			catch (UnauthorizedAccessException) { }
		}
	}

	/// <summary>
	/// My System.IO.Path extensions.
	/// </summary>
	/// <remarks>
	/// System.IO.Path is not OK due to invalid file system chars that are valid for other providers.
	/// </remarks>
	static class PathEx
	{
		public static bool IsPSFile(string fileName)
		{
			return
				fileName.EndsWith(".ps1", StringComparison.OrdinalIgnoreCase) ||
				fileName.EndsWith(".psm1", StringComparison.OrdinalIgnoreCase) ||
				fileName.EndsWith(".psd1", StringComparison.OrdinalIgnoreCase);
		}
		/// <summary>
		/// Does a string looks like a file system path?
		/// </summary>
		public static bool IsFSPath(string name)
		{
			return name.StartsWith("\\\\", StringComparison.Ordinal) || (name.Length > 3 && name[1] == ':');
		}
		public static string Combine(string path, string file)
		{
			if (path == null)
				return file;
			if (path.EndsWith("\\", StringComparison.Ordinal))
				return path + file;
			// 090824
			if (path.EndsWith("::", StringComparison.Ordinal))
				return path + file;
			else
				return path + "\\" + file;
		}
		public static string GetFileName(string path)
		{
			int i = path.LastIndexOf('\\');
			if (i < 0)
				return path;

			return path.Substring(i + 1);
		}
		public static string GetDirectoryName(string path)
		{
			int i = path.LastIndexOf('\\');
			if (i < 0)
				return string.Empty;

			return path.Substring(0, i);
		}
		/// <summary>
		/// Tries to recognize an existing file path by an object.
		/// </summary>
		/// <param name="value">Any object, e.g. FileInfo, String.</param>
		/// <returns>Existing file path or null.</returns>
		public static string TryGetFilePath(object value) //_091202_073429
		{
			FileInfo fi = PowerShellFar.Cast<FileInfo>.From(value);
			if (fi != null)
				return fi.FullName;

			string path;
			if (LanguagePrimitives.TryConvertTo<string>(value, out path))
			{
				// looks like a full path
				if (path.Length > 3 && path.Substring(1, 2) == ":\\" || path.StartsWith("\\\\", StringComparison.OrdinalIgnoreCase))
				{
					if (File.Exists(path))
						return path;
				}
			}

			return null;
		}
	}

	/// <summary>
	/// My System.Management.Automation.ProviderInfo extensions.
	/// </summary>
	static class ProviderInfoEx
	{
		public static bool HasContent(ProviderInfo provider)
		{
			return provider.ImplementingType.GetInterface("IContentCmdletProvider") != null;
		}
		public static bool HasDynamicProperty(ProviderInfo provider)
		{
			return provider.ImplementingType.GetInterface("IDynamicPropertyCmdletProvider") != null;
		}
		public static bool HasProperty(ProviderInfo provider)
		{
			return provider.ImplementingType.GetInterface("IPropertyCmdletProvider") != null;
		}
		public static bool IsNavigation(ProviderInfo provider)
		{
			//! 'is' does not work, because we work just on a type, not an instance
			return provider.ImplementingType.IsSubclassOf(typeof(NavigationCmdletProvider));
		}
	}

	static class ProcessEx
	{
		/// <summary>
		/// Just a wrapper and helper to watch calls.
		/// </summary>
		[EnvironmentPermissionAttribute(SecurityAction.LinkDemand, Unrestricted = true)]
		public static Process Start(string fileName, string arguments)
		{
			return Process.Start(new ProcessStartInfo() { FileName = fileName, Arguments = arguments });
		}
	}
}
