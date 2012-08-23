
/*
FarNet plugin for Far Manager
Copyright (c) 2005 FarNet Team
*/

using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;

namespace FarNet
{
	/// <summary>
	/// TEST ONLY
	/// </summary>
	public class XmlAttributeInfo
	{
		/// <summary>
		/// TEST ONLY
		/// </summary>
		public XmlAttributeInfo(string name, Func<object, object> getter)
		{
			Name = name;
			Getter = getter;
		}
		/// <summary>
		/// TEST ONLY
		/// </summary>
		public string Name { get; private set; }
		/// <summary>
		/// TEST ONLY
		/// </summary>
		public Func<object, object> Getter { get; private set; }
	}
	/// <summary>
	/// Not yet public.
	/// </summary>
	public interface IXmlInfo
	{
		///
		string XmlNodeName();
		///
		IList<XmlAttributeInfo> XmlAttributes();
	}
	/// <summary>
	/// Abstract panel item representing file, directory, plugin item, or module item.
	/// </summary>
	/// <remarks>
	/// Modules may implement derived classes in order to represent their panel
	/// items effectively. Alternatively, they may use <see cref="SetFile"/>,
	/// already implemented simple property set.
	/// <para>
	/// Although this class is abstract all its virtual properties are defined,
	/// they get default values and throw <c>NotImplementedException</c> on
	/// setting. Thus, derived classes do not have to implement every property,
	/// at least <see cref="Name"/> has to be defined (in order to be shown in
	/// a panel), other properties are implemented only if they are used.
	/// </para>
	/// </remarks>
	public abstract class FarFile : IXmlInfo
	{
		/// <summary>
		/// File name.
		/// </summary>
		public virtual string Name
		{
			get { return null; }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// Description.
		/// </summary>
		public virtual string Description
		{
			get { return null; }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// Owner.
		/// </summary>
		public virtual string Owner
		{
			get { return null; }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// User data. Only for <see cref="Panel"/>.
		/// </summary>
		public virtual object Data
		{
			get { return null; } //??? _090610_071700
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// Creation time.
		/// </summary>
		public virtual DateTime CreationTime
		{
			get { return new DateTime(); }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// Last access time.
		/// </summary>
		public virtual DateTime LastAccessTime
		{
			get { return new DateTime(); }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// Last write time.
		/// </summary>
		public virtual DateTime LastWriteTime
		{
			get { return new DateTime(); }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// File length.
		/// </summary>
		public virtual long Length
		{
			get { return 0; }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// Custom columns. See <see cref="PanelPlan"/>.
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Usage", "CA2227:CollectionPropertiesShouldBeReadOnly")]
		public virtual ICollection Columns
		{
			get { return null; }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// File attributes. All <c>Is*</c> properties are based on this value.
		/// </summary>
		/// <remarks>
		/// Derived class may override this property and cannot override <c>Is*</c>.
		/// All <c>Is*</c> properties are completely mapped to this value.
		/// </remarks>
		public virtual FileAttributes Attributes
		{
			get { return 0; }
			set { throw new NotImplementedException(); }
		}
		/// <summary>
		/// Read only attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsReadOnly
		{
			get { return (Attributes & FileAttributes.ReadOnly) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.ReadOnly) : (Attributes & ~FileAttributes.ReadOnly); }
		}
		/// <summary>
		/// Hidden attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsHidden
		{
			get { return (Attributes & FileAttributes.Hidden) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.Hidden) : (Attributes & ~FileAttributes.Hidden); }
		}
		/// <summary>
		/// System attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsSystem
		{
			get { return (Attributes & FileAttributes.System) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.System) : (Attributes & ~FileAttributes.System); }
		}
		/// <summary>
		/// Directory attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsDirectory
		{
			get { return (Attributes & FileAttributes.Directory) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.Directory) : (Attributes & ~FileAttributes.Directory); }
		}
		/// <summary>
		/// Archive attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsArchive
		{
			get { return (Attributes & FileAttributes.Archive) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.Archive) : (Attributes & ~FileAttributes.Archive); }
		}
		/// <summary>
		/// Reparse point attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsReparsePoint
		{
			get { return (Attributes & FileAttributes.ReparsePoint) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.ReparsePoint) : (Attributes & ~FileAttributes.ReparsePoint); }
		}
		/// <summary>
		/// Compressed attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsCompressed
		{
			get { return (Attributes & FileAttributes.Compressed) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.Compressed) : (Attributes & ~FileAttributes.Compressed); }
		}
		/// <summary>
		/// Encrypted attribute.
		/// See <see cref="Attributes"/>.
		/// </summary>
		public bool IsEncrypted
		{
			get { return (Attributes & FileAttributes.Encrypted) != 0; }
			set { Attributes = value ? (Attributes | FileAttributes.Encrypted) : (Attributes & ~FileAttributes.Encrypted); }
		}
		/// <summary>
		/// Returns the <see cref="Name"/>.
		/// </summary>
		public sealed override string ToString()
		{
			return Name;
		}
		/// <summary>
		/// Not yet public.
		/// </summary>
		public virtual string XmlNodeName()
		{
			return IsDirectory ? "Directory" : "File";
		}
		static ReadOnlyCollection<XmlAttributeInfo> _attrs;
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
		static ReadOnlyCollection<XmlAttributeInfo> XmlAttr()
		{
			if (_attrs != null)
				return _attrs;

			var attrs = new XmlAttributeInfo[]
			{
				new XmlAttributeInfo("Name", (object file) => ((FarFile)file).Name),
				new XmlAttributeInfo("Description", (object file) => ((FarFile)file).Description),
				new XmlAttributeInfo("Owner", (object file) => ((FarFile)file).Owner),
				new XmlAttributeInfo("Length", (object file) => ((FarFile)file).Length),
				new XmlAttributeInfo("CreationTime", (object file) => ((FarFile)file).CreationTime),
				new XmlAttributeInfo("LastAccessTime", (object file) => ((FarFile)file).LastAccessTime),
				new XmlAttributeInfo("LastWriteTime", (object file) => ((FarFile)file).LastWriteTime),
				new XmlAttributeInfo("ReadOnly", (object file) => ((FarFile)file).IsReadOnly),
				new XmlAttributeInfo("Hidden", (object file) => ((FarFile)file).IsHidden),
				new XmlAttributeInfo("System", (object file) => ((FarFile)file).IsSystem),
				new XmlAttributeInfo("Archive", (object file) => ((FarFile)file).IsArchive),
				new XmlAttributeInfo("Compressed", (object file) => ((FarFile)file).IsCompressed),
				new XmlAttributeInfo("ReparsePoint", (object file) => ((FarFile)file).IsReparsePoint),
			};

			_attrs = new ReadOnlyCollection<XmlAttributeInfo>(attrs);
			return _attrs;
		}
		/// <summary>
		/// Not yet public.
		/// </summary>
		public virtual IList<XmlAttributeInfo> XmlAttributes()
		{
			return XmlAttr();
		}
	}

	/// <summary>
	/// Straightforward implementation of <see cref="FarFile"/> ready to use by module panels.
	/// </summary>
	/// <remarks>
	/// It is just a set of properties where any property can be set. In most
	/// cases panels may use this class for their items. In some cases they may
	/// implement custom classes derived from <see cref="FarFile"/> in order to
	/// represent data more effectively (using less memory or working faster).
	/// </remarks>
	public sealed class SetFile : FarFile
	{
		/// <summary>
		/// Creates an empty file data object.
		/// </summary>
		public SetFile()
		{ }
		/// <summary>
		/// Creates file data snapshot from a <see cref="FarFile"/> object.
		/// </summary>
		/// <param name="file">Any panel file which data are taken.</param>
		public SetFile(FarFile file)
		{
			if (file == null)
				throw new ArgumentNullException("file");

			Attributes = file.Attributes;
			CreationTime = file.CreationTime;
			Data = file.Data;
			Description = file.Description;
			LastAccessTime = file.LastAccessTime;
			LastWriteTime = file.LastWriteTime;
			Length = file.Length;
			Name = file.Name;
			Owner = file.Owner;
		}
		/// <summary>
		/// Creates file data snapshot from a <see cref="FileSystemInfo"/> object.
		/// </summary>
		/// <param name="info">File system item info (normally <see cref="FileInfo"/> or <see cref="DirectoryInfo"/>).</param>
		/// <param name="fullName">Use the full name (path) as the name.</param>
		public SetFile(FileSystemInfo info, bool fullName)
		{
			if (info == null)
				throw new ArgumentNullException("info");

			Name = fullName ? info.FullName : info.Name;
			CreationTime = info.CreationTime;
			LastAccessTime = info.LastAccessTime;
			LastWriteTime = info.LastWriteTime;
			Attributes = info.Attributes;

			if ((Attributes & FileAttributes.Directory) == 0)
				Length = ((FileInfo)info).Length;
		}
		/// <summary>
		/// File name.
		/// </summary>
		public override string Name { get; set; }
		/// <summary>
		/// Description.
		/// </summary>
		public override string Description { get; set; }
		/// <summary>
		/// Owner.
		/// </summary>
		public override string Owner { get; set; }
		/// <summary>
		/// Creation time.
		/// </summary>
		public override DateTime CreationTime { get; set; }
		/// <summary>
		/// Last access time.
		/// </summary>
		public override DateTime LastAccessTime { get; set; }
		/// <summary>
		/// Last write time.
		/// </summary>
		public override DateTime LastWriteTime { get; set; }
		/// <summary>
		/// File length.
		/// </summary>
		public override long Length { get; set; }
		/// <summary>
		/// User data. Only for <see cref="Panel"/>.
		/// </summary>
		public override object Data { get; set; }
		/// <summary>
		/// File attributes.
		/// </summary>
		public override FileAttributes Attributes { get; set; }
		/// <summary>
		/// Custom columns. See <see cref="PanelPlan"/>.
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Usage", "CA2227:CollectionPropertiesShouldBeReadOnly")]
		public override ICollection Columns { get; set; }
	}

	/// <summary>
	/// File that wraps another file.
	/// </summary>
	public class WrapFile : FarFile
	{
		///
		public WrapFile(FarFile file)
		{
			if (file == null) throw new ArgumentNullException("file");
			_File = file;
		}
		/// <summary>
		/// Gets the source file.
		/// </summary>
		public FarFile File { get { return _File; } }
		readonly FarFile _File;
		///
		public override string Name { get { return File.Name; } }
		///
		public override string Description { get { return File.Description; } }
		///
		public override string Owner { get { return File.Owner; } }
		///
		public override object Data { get { return File.Data; } }
		///
		public override DateTime CreationTime { get { return File.CreationTime; } }
		///
		public override DateTime LastAccessTime { get { return File.LastAccessTime; } }
		///
		public override DateTime LastWriteTime { get { return File.LastWriteTime; } }
		///
		public override long Length { get { return File.Length; } }
		///
		public override ICollection Columns { get { return File.Columns; } }
		///
		public override FileAttributes Attributes { get { return File.Attributes; } }
	}

	/// <summary>
	/// Compares files by their references.
	/// </summary>
	public sealed class FileFileComparer : EqualityComparer<FarFile>
	{
		///
		public override bool Equals(FarFile x, FarFile y)
		{
			return object.Equals(x, y);
		}
		///
		public override int GetHashCode(FarFile obj)
		{
			return obj == null ? 0 : obj.GetHashCode();
		}
	}

	/// <summary>
	/// Compares files by their <see cref="FarFile.Data"/> references.
	/// </summary>
	public sealed class FileDataComparer : EqualityComparer<FarFile>
	{
		///
		public override bool Equals(FarFile x, FarFile y)
		{
			if (x == null || y == null)
				return x == null && y == null;
			else
				return object.ReferenceEquals(x.Data, y.Data);
		}
		///
		public override int GetHashCode(FarFile obj)
		{
			return obj == null || obj.Data == null ? 0 : obj.Data.GetHashCode();
		}
	}

	/// <summary>
	/// Compares files by their names.
	/// </summary>
	public sealed class FileNameComparer : EqualityComparer<FarFile>
	{
		readonly StringComparer _comparer;
		/// <summary>
		/// New comparer with the <c>OrdinalIgnoreCase</c> string comparer.
		/// </summary>
		public FileNameComparer()
		{
			_comparer = StringComparer.OrdinalIgnoreCase;
		}
		/// <summary>
		/// New comparer with the specified string comparer.
		/// </summary>
		public FileNameComparer(StringComparer comparer)
		{
			if (comparer == null) throw new ArgumentNullException("comparer");
			_comparer = comparer;
		}
		///
		public override bool Equals(FarFile x, FarFile y)
		{
			if (x == null || y == null)
				return x == null && y == null;
			else
				return _comparer.Equals(x.Name, y.Name);
		}
		///
		public override int GetHashCode(FarFile obj)
		{
			return obj == null || obj.Name == null ? 0 : obj.Name.GetHashCode();
		}
	}

}