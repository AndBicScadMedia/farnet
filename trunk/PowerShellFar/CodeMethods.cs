/*
PowerShellFar plugin for Far Manager
Copyright (C) 2006-2009 Roman Kuzmin
*/

using System;
using System.Collections.Generic;
using System.IO;
using System.Management.Automation;
using System.Text;
using Microsoft.Win32;

namespace PowerShellFar
{
	/// <summary>
	/// Infrastructure.
	/// </summary>
	public static class CodeMethods
	{
		/// <summary>
		/// Gets Far description for a FS item.
		/// </summary>
		public static string FileSystemInfoGetFarDescription(PSObject instance)
		{
			FileSystemInfo info = Convert<FileSystemInfo>.From(instance);
			if (info != null)
				return Description.Get(info.FullName);
			else
				return string.Empty;
		}

		/// <summary>
		/// Sets Far description for a FS item.
		/// </summary>
		public static void FileSystemInfoSetFarDescription(PSObject instance, string value)
		{
			FileSystemInfo info = Convert<FileSystemInfo>.From(instance);
			if (info != null)
				Description.Set(info.FullName, value);
		}
	}

	// Directorty item description map; used for caching directory descriptions.
	class DescriptionMap
	{
		public DescriptionMap(string directory, DateTime timestamp, Dictionary<string, string> map)
		{
			Directory = directory;
			Timestamp = timestamp;
			Map = map;
		}
		public string Directory { get; private set; }
		public DateTime Timestamp { get; private set; }
		public Dictionary<string, string> Map { get; private set; }
	}

	/// <summary>
	/// Far description tools for internal use.
	/// </summary>
	static class Description
	{
		// Default description file name
		const string DefaultDescriptionName = "Descript.ion";

		// Description file names from registry, at least one, and other data.
		static string[] _DescriptionNames;
		static string[] DescriptionNames { get { Init(); return _DescriptionNames; } }
		static bool _SetHidden;
		static bool SetHidden { get { Init(); return _SetHidden; } }

		// Directory description data cache, use locked!
		static readonly WeakReference WeakCache = new WeakReference(null);

		// Gets settings once.
		static void Init()
		{
			if (_DescriptionNames != null)
				return;

			// the key
			string keyName = Registry.CurrentUser.Name + "\\" + A.Far.RootFar + "\\Descriptions";

			// SetHidden
			_SetHidden = 0 != (int)Registry.GetValue(keyName, "SetHidden", 0);

			// ListNames
			string csv = (string)Registry.GetValue(keyName, "ListNames", DefaultDescriptionName);
			string[] names = csv.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
			if (names.Length == 0)
				_DescriptionNames = new string[] { DefaultDescriptionName };
			else
				_DescriptionNames = names;
		}

		// Gets full path of existing or default description file for a directory.
		internal static string GetDescriptionFile(string directory, out bool exists)
		{
			directory = Path.GetFullPath(directory);
			foreach (string name in DescriptionNames)
			{
				string path = Path.Combine(directory, name);
				if (File.Exists(path))
				{
					exists = true;
					return path;
				}
			}
			exists = false;
			return Path.Combine(directory, DescriptionNames[0]);
		}

		// Updates the description file and removes it if it is empty.
		internal static void UpdateDescriptionFile(string descriptionFile)
		{
			Dictionary<string, string> data = Import(descriptionFile);
			string directory = Path.GetDirectoryName(descriptionFile);
			string[] names = new string[data.Count];
			data.Keys.CopyTo(names, 0);
			foreach (string name in names)
			{
				string path = Path.Combine(directory, name);
				if (!File.Exists(path) && !Directory.Exists(path))
					data.Remove(name);
			}
			Export(descriptionFile, data);
		}

		static Dictionary<string, string> Import(string descriptionFile)
		{
			Dictionary<string, string> r = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
			if (!File.Exists(descriptionFile))
				return r;

			using (StreamReader sr = new StreamReader(descriptionFile, Encoding.GetEncoding(A.Far.Zoo.OemCP)))
			{
				string line;
				while ((line = sr.ReadLine()) != null)
				{
					string name, description;
					if (line.StartsWith("\"", StringComparison.Ordinal))
					{
						int i = line.IndexOf('"', 1);
						if (i < 0)
							continue;

						name = line.Substring(1, i - 1);
						description = line.Substring(i + 1).Trim();
					}
					else
					{
						int i = line.IndexOf(' ');
						if (i < 0)
							continue;

						name = line.Substring(0, i);
						description = line.Substring(i + 1).Trim();
					}

					r.Add(name, description);
				}
			}

			return r;
		}

		// Gets Far description for a FS item.
		internal static string Get(string path)
		{
			lock (WeakCache)
			{
				// strong cache ASAP
				DescriptionMap cache = (DescriptionMap)WeakCache.Target;

				// description file, if any
				string directory = Path.GetDirectoryName(path);
				bool exists;
				string descriptionFile = GetDescriptionFile(directory, out exists);
				if (!exists)
					return string.Empty;

				// update the cache
				if (cache == null || Kit.Compare(directory, cache.Directory) != 0 || File.GetLastWriteTime(descriptionFile) != cache.Timestamp)
				{
					cache = new DescriptionMap(directory, File.GetLastWriteTime(descriptionFile), Import(descriptionFile));
					WeakCache.Target = cache;
				}

				// description from cache, if any
				string value;
				return cache.Map.TryGetValue(Path.GetFileName(path), out value) ? value : string.Empty;
			}
		}

		// Sets Far description for a FS item.
		internal static void Set(string path, string value)
		{
			lock (WeakCache)
			{
				// get data and set new value
				string directory = Path.GetDirectoryName(path);
				bool exists;
				string descriptionFile = GetDescriptionFile(directory, out exists);
				Dictionary<string, string> map = Import(descriptionFile);
				map[Path.GetFileName(path)] = value.TrimEnd().Replace("\r\n", " _ ").Replace("\r", " _ ");

				// export
				Export(descriptionFile, map);
			}
		}

		//! *** CHANGE CAREFULLY AND TEST WELL
		//! It may fail if another process operates on the file, too.
		//! But it seems to be rare and massive data loss is unlikely.
		//! NB: approach with a temporary file is slower and less stable.
		static void Export(string descriptionFile, Dictionary<string, string> map)
		{
			lock (WeakCache)
			{
				// sort by file name
				string[] keys = new string[map.Count];
				map.Keys.CopyTo(keys, 0);
				Array.Sort(keys, StringComparer.OrdinalIgnoreCase);

				// get existing file info
				FileAttributes attr = 0;
				bool existed = File.Exists(descriptionFile);
				if (existed)
				{
					//! Killing the file now is much less stable.
					attr = File.GetAttributes(descriptionFile);
					if (0 != (attr & (FileAttributes.ReadOnly | FileAttributes.System | FileAttributes.Hidden)))
						File.SetAttributes(descriptionFile, attr & ~(FileAttributes.ReadOnly | FileAttributes.System | FileAttributes.Hidden));
				}

				// write
				int nWritten = 0;
				using (StreamWriter sw = new StreamWriter(descriptionFile, false, Encoding.GetEncoding(A.Far.Zoo.OemCP)))
				{
					foreach (string key in keys)
					{
						string description = map[key];
						if (string.IsNullOrEmpty(description))
							continue;

						++nWritten;
						if (key.IndexOf(' ') >= 0)
							sw.WriteLine("\"" + key + "\" " + description);
						else
							sw.WriteLine(key + " " + description);
					}
				}

				// case: nothing is written
				if (nWritten == 0)
				{
					File.Delete(descriptionFile);
					WeakCache.Target = null;
					return;
				}

				// restore attributes + Archive
				if (existed)
					File.SetAttributes(descriptionFile, attr | FileAttributes.Archive);
				// add hidden attribute for a new file
				else if (SetHidden)
					File.SetAttributes(descriptionFile, File.GetAttributes(descriptionFile) | FileAttributes.Hidden);

				// cache
				WeakCache.Target = new DescriptionMap(Path.GetDirectoryName(descriptionFile), File.GetLastWriteTime(descriptionFile), map); 
			}
		}

	}
}