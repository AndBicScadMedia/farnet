
/*
PowerShellFar module for Far Manager
Copyright (c) 2006-2015 Roman Kuzmin
*/

using System.Collections.Generic;
using FarNet;

namespace PowerShellFar
{
	/// <summary>
	/// Formatted table panel.
	/// </summary>
	public abstract class FormatPanel : TablePanel
	{
		/// <summary>
		/// Gets the panel explorer.
		/// </summary>
		public new FormatExplorer Explorer { get { return (FormatExplorer)base.Explorer; } }
		/// <summary>
		/// New format panel.
		/// </summary>
		/// <param name="explorer">The panel explorer.</param>
		protected FormatPanel(FormatExplorer explorer) : base(explorer) { }
		/// <summary>
		/// Gets a list of ready files or a collection of PS objects.
		/// </summary>
		internal override string HelpMenuTextOpenFileMembers { get { return "Object members"; } }
		internal void BuildPlan(string sameType)
		{
			PanelPlan plan = GetPlan(PanelViewMode.AlternativeFull);
			if (plan == null)
				plan = new PanelPlan();

			// choose columns
			if (sameType == null)
			{
				//! The long "Index" clashes to sort order mark, use the short "##"
				plan.Columns = new FarColumn[]
				{
				new SetColumn() { Kind = "S", Name = "##"},
				new SetColumn() { Kind = "N", Name = "Value"},
				new SetColumn() { Kind = "Z", Name = "Type"}
				};
			}
			else
			{
				plan.Columns = new FarColumn[]
				{
				new SetColumn() { Kind = "N", Name = sameType }
				};
			}

			SetPlan(PanelViewMode.AlternativeFull, plan);
		}
		/// <inheritdoc/>
		public override void Open()
		{
			if (IsOpened)
				return;

			if (Explorer.Metas != null)
				SetPlan(PanelViewMode.AlternativeFull, Format.SetupPanelMode(Explorer.Metas));

			base.Open();
		}
		/// <inheritdoc/>
		public override IList<FarFile> UIGetFiles(GetFilesEventArgs args)
		{
			if (args == null) return null;

			args.Parameter = this;
			
			return base.UIGetFiles(args);
		}
	}
}
