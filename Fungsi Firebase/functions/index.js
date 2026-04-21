/* eslint-disable require-jsdoc */
/* eslint-disable arrow-parens */
/* eslint-disable no-multi-spaces */
/* eslint-disable object-curly-spacing */
/* eslint-disable quotes */
/* eslint-disable max-len */
const functions = require("firebase-functions/v1");
const admin = require("firebase-admin");
const qrcode = require("qrcode"); // qrcode library
const sharp = require("sharp");
const {getDownloadURL} = require("firebase-admin/storage");
const {error} = require("firebase-functions/logger");

const accountSid = "your-account-sid";
const authToken = "your-auth-tokenn";
const client = require("twilio")(accountSid, authToken);

const mqtt = require('mqtt');

admin.initializeApp();

exports.saveQrString = functions.database
    .ref('/Locker List/{lockerId}/WA Number')
    .onUpdate((change, context) => {
      const whatsappNumber = change.after.val();
      const lockerId = context.params.lockerId;
      console.log('Locker Get:', lockerId);

      if (whatsappNumber != 0) {
        const qrRandString = generateQrString();
        setQrString(qrRandString, lockerId);
      } else { // new
        const LockerStatus = 1;
        const saveStatusPath = admin.database().ref(`/Locker List/${lockerId}/Status`);
        saveStatusPath.set(LockerStatus);
        console.log('Status changed');
      }
    });

exports.QrCodeProcessing = functions.database
    .ref('/Locker List/{lockerId}/Qr String')
    .onUpdate((change, context) => {
      const qrRandString = change.after.val();
      const lockerId = context.params.lockerId;

      console.log('Locker Get:', lockerId);
      if (qrRandString.length == 26) { // new
        generateToSend(qrRandString, lockerId);
      }
    });

exports.qrAuthentication = functions.database
    .ref('/Locker List/{lockerId}/Qr Key')
    .onUpdate((change, context) => {
      const userQrKey = change.after.val();

      if (userQrKey.length == 26) { // new
        const lockerId = context.params.lockerId;
        console.log('Locker Get:', lockerId);
        qrKeyStringCompare(userQrKey, lockerId);
      }
    });

exports.mqttProcessing = functions.database
    .ref('/Locker List/{lockerId}/Lock')
    .onUpdate((change, context) => {
      const lockerId = context.params.lockerId;
      const lockerLock = change.after.val();

      const client = mqtt.connect('your-cluster', {
        username: 'your-username',
        password: 'your-password',
      });

      client.on('connect', () => {
        console.log('Connected to MQTT broker');
        console.log('Path get:', lockerId);
        client.publish(`/${lockerId}/Lock`, String(lockerLock), { retain: true, qos: 1 });
      });

      console.log('Lock MQTT Send');
      statusBasedLock(lockerLock, lockerId);

      client.on('error', (err) => {
        console.error('Error connecting to MQTT broker:', err);
      });
    });

function generateQrString() {
  const characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  let result = "";
  const length = characters.length;

  // Generate 26 random characters
  for (let i = 0; i < 26; i++) {
    result += characters.charAt(Math.floor(Math.random() * length));
  }
  return result;
}

async function setQrString(qrRandString, lockerId) {
  try {
    // --------------------------Change Availability Status-------------------//
    const LockerStatus = 0;
    const saveStatusPath = admin.database().ref(`/Locker List/${lockerId}/Status`);
    await saveStatusPath.set(LockerStatus);
    console.log('Status changed');

    // ---------------------------Save Random String--------------------------//
    const saveNumber = admin.database().ref(`/Locker List/${lockerId}/Qr String`);
    await saveNumber.set(qrRandString);
    console.log(`Saved Qr String`);
  } catch (error) {
    console.error("Error Saving Qr String:", error);
  }
}

async function generateToSend(qrRandString, lockerId) {
  try {
    // -----------------------Generating Qr Image function-----------------------//
    const qrOptions = {
      errorCorrectionLevel: "H",
      margin: 4,
      cellSize: 100,
    };
    const imageBuffer =  await qrcode.toBuffer(qrRandString, qrOptions);
    const scaledBuffer = await sharp(imageBuffer).resize({ width: 1026, height: 1026 }).toBuffer();
    console.log('Generate Qr Image successfully!');

    // --------------------------Saving Image to storage function-------------------------//
    const fileName = `${lockerId}` + ` Key` + `.png`;
    const bucket = admin.storage().bucket('smart-locker-demo');
    const fileRef = bucket.file(fileName);
    await fileRef.save(scaledBuffer, { contentType: 'image/png' });
    const url = await getDownloadURL(fileRef);
    console.log('QR code image uploaded successfully!');
    console.log('Donwload URL:', url);
    const prefixLength = 'https://firebasestorage.googleapis.com/'.length;
    const urlSlicedPath = await url.slice(prefixLength);
    console.log('Sliced Path:', urlSlicedPath);

    // ---------------------------Twilio message Function-----------------------//
    const waNumberPath = await admin.database().ref(`/Locker List/${lockerId}/WA Number`).once('value');
    const waNumberData = await waNumberPath.val();
    console.log('Get Number:', waNumberData);
    const messageBody = `Anda Meminjam ${lockerId}. Berikut Kode Qr sebagai kunci loker anda:`;
    await client.messages
        .create({
          // contentSid: "HXdf76db13d2a09fda9ed717d2e754500c",
          // contentVariables: JSON.stringify({
          //   1: "Loker 1",
          //   2: "v0/b/smart-locker-demo/o/lockerLocker%201.png?alt=media&token=d8c87393-56c4-4d5f-8c52-34f6377d3a36",
          // }),
          body: messageBody,
          // from: "whatsapp:+19382533873", // Replace with your Twilio WhatsApp number (with country code)
          from: "whatsapp:+14155238886", // Replace with your Twilio WhatsApp number (with country code)
          // from: "whatsapp:+17175239092", // Replace with your Twilio WhatsApp number (with country code)
          // messagingServiceSid: "MGcaa30a2678346a6658e90331d62b611c",
          mediaUrl: url,
          to: `whatsapp:+62` + waNumberData, // Ensure phone number includes country code
          // to: `whatsapp:+12018770916`, // Ensure phone number includes country code
        });
    console.log('Message Sent to:', waNumberData);

    // --------------------------Change Lock Open/close----------------------//
    const LockerLock = 1;
    const saveLockPath = admin.database().ref(`/Locker List/${lockerId}/Lock`);
    await saveLockPath.set(LockerLock);
    console.log('Lock changed');
  } catch (error) {
    console.error("Error:", error);

    // --------------------------Change Availability Status-------------------//
    const LockerStatus = 1;
    const saveStatusPath = admin.database().ref(`/Locker List/${lockerId}/Status`);
    await saveStatusPath.set(LockerStatus);
    console.log('Status changed');
  }
}

async function qrKeyStringCompare(userQrKey, lockerId) {
  try {
    const qrStringPath = await admin.database().ref(`/Locker List/${lockerId}/Qr String`).once('value');
    const qrStringData = await qrStringPath.val();
    console.log('Get Qr String:', qrStringData);

    const lockPath = await admin.database().ref(`/Locker List/${lockerId}/Lock`).once('value');
    const lockData = await lockPath.val();
    console.log('Get Lock:', lockData);

    if (qrStringData == userQrKey && lockData == 0) {
      console.log('Qr Key Valid:', qrStringData);
      // -------------------------notify user locker open---------------------//
      const waNumberPath = await admin.database().ref(`/Locker List/${lockerId}/WA Number`).once('value');
      const waNumberData = await waNumberPath.val();
      console.log('Get Number:', waNumberData);
      const messageBody = `Loker Terbuka. Terima Kasih telah meminjam loker`;
      await client.messages
          .create({
            body: messageBody,
            from: "whatsapp:+14155238886", // Replace with your Twilio WhatsApp number (with country code)
            to: `whatsapp:+62` + waNumberData, // Ensure phone number includes country code
          });
      console.log('Message Sent to:', waNumberData);
      // -------------------------change wa number to empty---------------------//
      const LockerWANumb = 0;
      const saveWaNumberPath = admin.database().ref(`/Locker List/${lockerId}/WA Number`);
      await saveWaNumberPath.set(LockerWANumb);
      console.log(`Saved Qr String`);
      // -------------------------change lock to open---------------------------//
      const LockerLock = 1;
      const saveLockPath = admin.database().ref(`/Locker List/${lockerId}/Lock`);
      await saveLockPath.set(LockerLock);
      console.log('Lock changed');
      // -------------------------change qr string to empty---------------------//
      const LockerString = 0;
      const saveQrStringPath = admin.database().ref(`/Locker List/${lockerId}/Qr String`);
      await saveQrStringPath.set(LockerString);
      console.log(`Saved Qr String`);
    } else {
      throw new Error("QR Code Key Does Not Match");
    }
  } catch (error) {
    console.error("Error Comparing Qr:", error);

    // -------------------------change Qr Key to empty---------------------//
    const LockerQrKey = 0;
    const qrKeyPath = admin.database().ref(`/Locker List/${lockerId}/Qr Key`);
    await qrKeyPath.set(LockerQrKey);
    console.log(`Saved Qr Key`);

    // ------------------------give error message whatsapp--------------------//
    const waNumberPath = await admin.database().ref(`/Locker List/${lockerId}/WA Number`).once('value');
    const waNumberData = await waNumberPath.val();
    console.log('Get Number:', waNumberData);
    const messageBody = `Kunci Kode QR anda salah atau loker masih terbuka`;
    await client.messages
        .create({
          body: messageBody,
          from: "whatsapp:+14155238886", // Replace with your Twilio WhatsApp number (with country code)
          to: `whatsapp:+62` + waNumberData, // Ensure phone number includes country code
        });
    console.log('Message Sent to:', waNumberData);
  }
}

async function statusBasedLock(lockerLock, lockerId) {
  const waNumberPath = await admin.database().ref(`/Locker List/${lockerId}/WA Number`).once('value');
  const waNumberData = await waNumberPath.val();
  console.log('Get Number:', waNumberData);
  console.log(`Saved Qr String`);
  if (waNumberData == 0 && lockerLock == 0) {
    // -------------------------change status to available---------------------//
    const LockerStatus = 1;
    const saveStatusPath = admin.database().ref(`/Locker List/${lockerId}/Status`);
    await saveStatusPath.set(LockerStatus);
    console.log('Status changed');
  }
}
