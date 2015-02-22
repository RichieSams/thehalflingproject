using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Net;
using System.Windows.Forms;

namespace LibDownloader {
	public partial class Downloader : Form {
		public Downloader() {
			InitializeComponent();

			m_webClient = new WebClient();
			m_webClient.DownloadProgressChanged += ProgressChanged;
			m_webClient.DownloadFileCompleted += Completed;
			m_webClient.Proxy = null;

			m_stopwatch = new Stopwatch();

			m_currentDownload = null;
		}

		private WebClient m_webClient;
		private Stopwatch m_stopwatch;

		private Queue<Tuple<Uri, string>> m_filesToDownload;
		private Tuple<Uri, string> m_currentDownload;
		private int m_fileSize;

		private void OnShown(object sender, EventArgs e) {
			// Read the files to download
			ReadBinaryLocations();

			// Start the first download
			Completed(null, null);
		}

		private void ProgressChanged(object sender, DownloadProgressChangedEventArgs e) {
			var percentage = (int)((float)e.BytesReceived / m_fileSize * 100);

			// Update the download label
			m_progressLabel.Text = string.Format("{0}%", percentage);

			// Update the progressbar percentage only when the value is not the same.
			m_progressBar.Value = percentage;
		}

		private void Completed(object sender, AsyncCompletedEventArgs e) {
			m_stopwatch.Stop();
			m_stopwatch.Reset();

			
			if (m_currentDownload != null) {
				// Decompress the files
				m_console.Text += "Extracting: " + m_currentDownload.Item2 + "\n";
				m_console.ScrollToCaret();
				using (var archive = ZipFile.OpenRead(m_currentDownload.Item2)) {
					foreach (var file in archive.Entries) {
						var completeFileName = Path.Combine("libs", file.FullName);

						// Assuming Empty for Directory
						if (file.Name == "") {
							Directory.CreateDirectory(Path.GetDirectoryName(completeFileName));
							continue;
						}

						file.ExtractToFile(completeFileName, true);
					}
				}

				// Delete the archive
				File.Delete(m_currentDownload.Item2);
			}

			if (m_filesToDownload.Count == 0) {
				m_console.Text += "Done\n";
				m_console.ScrollToCaret();
				return;
			}

			m_currentDownload = m_filesToDownload.Dequeue();

			var req = WebRequest.Create(m_currentDownload.Item1);
			req.Proxy = null;
			req.Method = "HEAD";
			using (var resp = req.GetResponse()) {
				m_fileSize = int.Parse(resp.Headers.Get("Content-Length"));
			}

			m_console.Text += "Downloading: " + m_currentDownload.Item2 + "\n";
			m_console.ScrollToCaret();
			m_stopwatch.Start();

			m_webClient.DownloadFileAsync(m_currentDownload.Item1, m_currentDownload.Item2);
		}

		private void ReadBinaryLocations() {
			m_filesToDownload = new Queue<Tuple<Uri, string>>();

			using (var reader = new StreamReader(new FileStream("libs\\binary_download_locations.txt", FileMode.Open))) {
				string line;

				while ((line = reader.ReadLine()) != null) {
					var splitLine = line.Split(new[] {"=>", " "}, StringSplitOptions.RemoveEmptyEntries);

					m_filesToDownload.Enqueue(new Tuple<Uri, string>(new Uri(splitLine[0]), splitLine[1]));
				}
			}
		}

	}
}
