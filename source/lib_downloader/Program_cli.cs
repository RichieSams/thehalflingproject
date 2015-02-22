using System;
using System.Collections.Generic;
using System.IO;

namespace LibDownloader_cli {
	static class Program {
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() {
			var downloader = new FileDownloader();

			downloader.ReadBinaryLocations();
			downloader.DownloadFiles();
		}
	}
}
