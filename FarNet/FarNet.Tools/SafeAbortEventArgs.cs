﻿
// http://www.codeproject.com/KB/threads/AbortIfSafe.aspx

using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading;

namespace Pfz.Threading
{
	/// <summary>
	/// Determines how SafeAbort.AbortIfSafe method will verify for safety.
	/// </summary>
	enum SafeAbortMode
	{
		/// <summary>
		/// All validations, including user-registered validations, will be used.
		/// This has all the guarantees over using keyword and IDisposable objects, and also runs
		/// the SafeAbort.Validating event, to check for some user function that is not returning
		/// an IDisposable object but may be changing global data, which must be reverted.
		/// </summary>
		RunAllValidations,
		/// <summary>
		/// Only user validations will be ignored. This will guarantee that all "using" blocks will work,
		/// or any code that creates a disposable object, does a try, and finalizes it.
		/// </summary>
		IgnoreUserValidations,
		/// <summary>
		/// This will only guarantee that IDisposable objects will be fully created, but even an using block may fail,
		/// as the abort can happen between the object construction and its store to a local variable.
		/// </summary>
		AllowUsingsToFail
	}
	/// <summary>
	/// Argument passed to SafeAbort.Verifying when AbortIfSafe is still unsure if it is safe
	/// to abort the thread and is able to run user validations.
	/// </summary>
	class SafeAbortEventArgs : EventArgs
	{
		internal SafeAbortEventArgs(Thread thread, StackTrace stackTrace, StackFrame[] stackFrames)
		{
			Thread = thread;
			StackTrace = stackTrace;
			StackFrames = new ReadOnlyCollection<StackFrame>(stackFrames);
			CanAbort = true;
		}
		/// <summary>
		/// Gets the Thread that is about to be aborted.
		/// </summary>
		public Thread Thread { get; private set; }
		/// <summary>
		/// Gets the StackTrace of the thread about to be aborted.
		/// </summary>
		public StackTrace StackTrace { get; private set; }
		/// <summary>
		/// Get the StackFrames of the thread to be aborted.
		/// </summary>
		public ReadOnlyCollection<StackFrame> StackFrames { get; private set; }
		/// <summary>
		/// Gets or sets a value indicating if the thread can be aborted.
		/// </summary>
		public bool CanAbort { get; set; }
	}
}