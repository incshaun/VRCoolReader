﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

/*
 * Copy application files to a generally available area. This ensures that
 * assets are available as general purpose files, even on mobile packaged applications.
 */
public class ExtractPackagedFiles : MonoBehaviour {
  
  void Start () {
    // List all files required here. These are all relative to the StreamingAssets directory.
    StartCoroutine (extractFile ("", "Credits.txt"));
    StartCoroutine (extractFile ("Books/Alice/", "pg11-images.epub"));
    StartCoroutine (extractFile ("Books/Dorian/", "pg174-images.epub"));
    StartCoroutine (extractFile ("Books/PeterRabbit/", "pg14838-images.epub"));
    StartCoroutine (extractFile ("Books/FormatSamples/", "FB2 The Golden Spears - Edmund Leamy.fb2"));
    StartCoroutine (extractFile ("Books/FormatSamples/", "MOBI The Golden Spears - Edmund Leamy.mobi"));
    StartCoroutine (extractFile ("Books/FormatSamples/", "PDB The Golden Spears - Edmund Leamy.pdb"));
    StartCoroutine (extractFile ("Books/FormatSamples/", "PDF The Golden Spears - Edmund Leamy.pdf"));
    StartCoroutine (extractFile ("Books/FormatSamples/", "RTF The Golden Spears - Edmund Leamy.rtf"));
    StartCoroutine (extractFile ("Books/FormatSamples/", "TCR The Golden Spears - Edmund Leamy.tcr"));
    StartCoroutine (extractFile ("Books/FormatSamples/", "TXT The Golden Spears - Edmund Leamy.txt"));
  }
  
  // Copy file from the android package to a readable/writeable region of the host file system.
  IEnumerator extractFile (string assetPath, string assetFile)
  {
    // Source is the streaming assets path.
    string sourcePath = Application.streamingAssetsPath + "/" + assetPath + assetFile;
    if ((sourcePath.Length > 0) && (sourcePath[0] == '/'))
    {
      sourcePath = "file://" + sourcePath;
    }
    string destinationPath = Application.persistentDataPath + "/" + assetFile;
    
    // Files must be handled via a WWW to extract.
    WWW w = new WWW (sourcePath);
    yield return w;
    File.WriteAllBytes (destinationPath, w.bytes);
//     Debug.Log (sourcePath + " -> " + destinationPath + " " + w.bytes.Length);
  }
}
