namespace LibDownloader {
	partial class Downloader {
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing) {
			if (disposing && (components != null)) {
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent() {
			this.m_progressBar = new System.Windows.Forms.ProgressBar();
			this.m_console = new System.Windows.Forms.RichTextBox();
			this.m_progressLabel = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// m_progressBar
			// 
			this.m_progressBar.Location = new System.Drawing.Point(12, 160);
			this.m_progressBar.Name = "m_progressBar";
			this.m_progressBar.Size = new System.Drawing.Size(440, 30);
			this.m_progressBar.TabIndex = 0;
			// 
			// m_console
			// 
			this.m_console.Location = new System.Drawing.Point(12, 12);
			this.m_console.Name = "m_console";
			this.m_console.ReadOnly = true;
			this.m_console.Size = new System.Drawing.Size(440, 142);
			this.m_console.TabIndex = 1;
			this.m_console.Text = "";
			// 
			// m_progressLabel
			// 
			this.m_progressLabel.AutoSize = true;
			this.m_progressLabel.Location = new System.Drawing.Point(220, 169);
			this.m_progressLabel.Name = "m_progressLabel";
			this.m_progressLabel.Size = new System.Drawing.Size(24, 13);
			this.m_progressLabel.TabIndex = 2;
			this.m_progressLabel.Text = "0 %";
			// 
			// Downloader
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(464, 202);
			this.Controls.Add(this.m_progressLabel);
			this.Controls.Add(this.m_console);
			this.Controls.Add(this.m_progressBar);
			this.Name = "Downloader";
			this.Text = "The Halfling Project - Required Libs Downloader";
			this.Shown += new System.EventHandler(this.OnShown);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ProgressBar m_progressBar;
		private System.Windows.Forms.RichTextBox m_console;
		private System.Windows.Forms.Label m_progressLabel;
	}
}