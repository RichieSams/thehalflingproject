
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Net;

namespace LibDownloader_cli {

	public class FileDownloader {
		private Queue<Tuple<Uri, string>> m_filesToDownload;
		private WebClient m_webClient;

		public FileDownloader() {
			m_webClient = new WebClient { Proxy = null };
		}


		public void ReadBinaryLocations() {
			m_filesToDownload = new Queue<Tuple<Uri, string>>();

			using (var reader = new StreamReader(new FileStream("binary_download_locations.txt", FileMode.Open))) {
				string line;

				while ((line = reader.ReadLine()) != null) {
					var splitLine = line.Split(new[] { "=>", " " }, StringSplitOptions.RemoveEmptyEntries);

					m_filesToDownload.Enqueue(new Tuple<Uri, string>(new Uri(splitLine[0]), splitLine[1]));
				}
			}
		}

		public void DownloadFiles() {
			while (m_filesToDownload.Count > 0) {
				var currentDownload = m_filesToDownload.Dequeue();

				// Download the file
				Console.WriteLine("Downloading: " + currentDownload.Item2);
				m_webClient.DownloadFile(currentDownload.Item1, currentDownload.Item2);

				// Decompress it
				Console.WriteLine("Decompressing: " + currentDownload.Item2);
				using (var archive = ZipFile.OpenRead(currentDownload.Item2)) {
					foreach (var file in archive.Entries) {
						// Assuming Empty for Directory
						if (file.Name == "") {
							Directory.CreateDirectory(Path.GetDirectoryName(file.FullName));
							continue;
						}

						file.ExtractToFile(file.FullName, true);
					}
				}

				// Cleanup
				File.Delete(currentDownload.Item2);
			}
		}
	}

}