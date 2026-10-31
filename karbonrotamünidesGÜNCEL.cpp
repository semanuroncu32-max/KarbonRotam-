#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* KARbonROTAM - KAMPUS KARBON TAKIP SISTEMI (KTU) */

/* EMISYON FAKTORLERI */
#define OTOBUS_EF 0.08
#define MINIBUS_EF 0.12
#define ARAC_EF 0.18
#define YURUYUS_EF 0.0
#define BISIKLET_EF 0.0

#define MAX_AY 12
#define MAX_OGRENCI 50
#define AGAC_YILLIK_CO2 21.0

/* AYLIK KARBON VERILERI */
typedef struct {
    int ay;
    int ulasimTuru;
    float mesafe;
    int haftalikGun;
    int yolcuSayisi;
    float aylikMesafe;
    float karbonSalimi;
} KarbonVerisi;

/* OGRENCI BILGILERI */
typedef struct {
    char ad[50];
    int ogrenciNo;
    char fakulte[60];
    KarbonVerisi veriler[MAX_AY];
    int kayitSayisi;
    float hedefYuzde;
    int yesilPuan;
} Ogrenci;

/* GLOBAL DEGISKENLER */
Ogrenci ogrenciler[MAX_OGRENCI];
int ogrenciSayisi = 0;
int aktifOgrenciIndex = -1;

/* FONKSIYON PROTOTIPLERI */
void ogrenciKayit();
void ogrenciSecGiris();
void ogrenciBilgisi();
void ulasimBilgisiAl();
float mesafeHesapla(float mesafe, int gun);
float karbonHesapla(int tur, float mesafe, int yolcu);
void aylikRapor();
void hedefBelirle();
void hedefKontrol();
void aylariKarsilastir();
void oneriler();
void agacKarsiligiHesapla();
void kampusKarbonHaritasi();
void gununYesilGorevi();
void sanalKampusOrmani();
void rozetVeUnvanlariGoster();
void yesilPazarYeri();
void dosyayaKaydet();
void dosyadanOku();
void menuGoster();
void tumKayitlariGoster();
float faktorBul(int tur);

/* EMISYON FAKTORUNU BUL */
float faktorBul(int tur)
{
    switch (tur) {
        case 1: return OTOBUS_EF;
        case 2: return MINIBUS_EF;
        case 3: return ARAC_EF;
        case 4: return YURUYUS_EF;
        case 5: return BISIKLET_EF;
        default: return -1;
    }
}

/* 1. YENI OGRENCI KAYDI */
void ogrenciKayit()
{
    if (ogrenciSayisi >= MAX_OGRENCI) {
        printf("\nMaksimum ogrenci sayisina ulasildi!\n");
        return;
    }

    printf("\n--- YENI OGRENCI KAYDI ---\n");
    printf("Ogrenci Adi: ");
    scanf(" %49[^\n]", ogrenciler[ogrenciSayisi].ad);

    printf("Ogrenci Numarasi: ");
    scanf("%d", &ogrenciler[ogrenciSayisi].ogrenciNo);

    printf("Fakulte / Bolum Adi (Orn: Of Teknoloji Fakultesi): ");
    scanf(" %59[^\n]", ogrenciler[ogrenciSayisi].fakulte);

    ogrenciler[ogrenciSayisi].kayitSayisi = 0;
    ogrenciler[ogrenciSayisi].hedefYuzde = 15.0;
    ogrenciler[ogrenciSayisi].yesilPuan = 0;

    aktifOgrenciIndex = ogrenciSayisi;
    ogrenciSayisi++;

    dosyayaKaydet();
    printf("\nKayit basarili! Otomatik olarak bu ogrenci ile giris yapildi.\n");
}

/* 2. OGRENCI DEGISTIR / SEC */
void ogrenciSecGiris()
{
    int i, secim;
    if (ogrenciSayisi == 0) {
        printf("\nKayitli ogrenci yok. Once ogrenci kaydi yapmalisiniz!\n");
        return;
    }

    printf("\n--- KAYITLI OGRENCILER ---\n");
    for (i = 0; i < ogrenciSayisi; i++) {
        printf("%d. Adi: %s | No: %d | Bolum: %s\n", i + 1, ogrenciler[i].ad, ogrenciler[i].ogrenciNo, ogrenciler[i].fakulte);
    }

    printf("Giris yapmak istediginiz ogrencinin sirasini secin (1-%d): ", ogrenciSayisi);
    scanf("%d", &secim);

    if (secim < 1 || secim > ogrenciSayisi) {
        printf("Gecersiz secim!\n");
        return;
    }

    aktifOgrenciIndex = secim - 1;
    printf("\nBasariyla '%s' adli ogrenci hesaba gecildi.\n", ogrenciler[aktifOgrenciIndex].ad);
}

/* 3. AKTIF OGRENCI BILGISI */
void ogrenciBilgisi()
{
    int i;
    float toplamKarbon = 0;

    if (aktifOgrenciIndex == -1) {
        printf("\nOnce ogrenci kaydi yapmali veya giris yapmalisiniz!\n");
        return;
    }

    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    for (i = 0; i < o->kayitSayisi; i++) {
        toplamKarbon += o->veriler[i].karbonSalimi;
    }

    printf("\n--- AKTIF OGRENCI BILGILERI ---\n");
    printf("Ad: %s\n", o->ad);
    printf("Ogrenci No: %d\n", o->ogrenciNo);
    printf("Fakulte / Bolum: %s\n", o->fakulte);
    printf("Kayitli ay sayisi: %d\n", o->kayitSayisi);
    printf("Toplam Karbon Saliminiz: %.3f kg CO2e\n", toplamKarbon);
    printf("Yesil Puaniniz: %d Puan\n", o->yesilPuan);
}

/* AYLIK MESAFE HESAPLAMA */
float mesafeHesapla(float mesafe, int gun)
{
    return mesafe * 2 * gun * 4.33;
}

/* KARBON SALIMI HESAPLAMA */
float karbonHesapla(int tur, float mesafe, int yolcu)
{
    float faktor = faktorBul(tur);
    if (faktor < 0) return -1;
    if (tur == 3) {
        if (yolcu < 1) return -1;
        faktor = faktor / yolcu;
    }
    return mesafe * faktor;
}

/* 4. ULASIM BILGISI GIRISI */
void ulasimBilgisiAl()
{
    int ay, tur, gun, yolcu, i;
    float mesafe, aylikMesafe, karbon;

    if (aktifOgrenciIndex == -1) {
        printf("\nIslem yapmak icin once ogrenci kaydi yapmali veya secmelisiniz!\n");
        return;
    }

    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    printf("\n--- ULASIM BILGISI GIRISI (%s) ---\n", o->ad);
    printf("Ay numarasi (1-12): ");
    scanf("%d", &ay);

    if (ay < 1 || ay > 12) {
        printf("Hatali ay numarasi!\n");
        return;
    }

    printf("\nULASIM TURU\n1- Otobus\n2- Minibus\n3- Ozel arac\n4- Yuruyus\n5- Bisiklet\nSeciminiz: ");
    scanf("%d", &tur);

    if (tur < 1 || tur > 5) {
        printf("Hatali ulasim turu!\n");
        return;
    }

    printf("Tek yon mesafe (km): ");
    scanf("%f", &mesafe);

    printf("Haftalik kampuse gelis gunu: ");
    scanf("%d", &gun);

    if (mesafe < 0 || gun < 0 || gun > 7) {
        printf("Hatali mesafe veya gun!\n");
        return;
    }

    yolcu = 1;
    if (tur == 3) {
        printf("Aractaki toplam yolcu sayisi: ");
        scanf("%d", &yolcu);
        if (yolcu < 1) {
            printf("Yolcu sayisi en az 1 olmalidir!\n");
            return;
        }
    }

    aylikMesafe = mesafeHesapla(mesafe, gun);
    karbon = karbonHesapla(tur, aylikMesafe, yolcu);

    for (i = 0; i < o->kayitSayisi; i++) {
        if (o->veriler[i].ay == ay) {
            break;
        }
    }

    if (i == o->kayitSayisi) {
        if (o->kayitSayisi >= MAX_AY) {
            printf("Maksimum kayit sayisina ulasildi!\n");
            return;
        }
        o->kayitSayisi++;
    }

    o->veriler[i].ay = ay;
    o->veriler[i].ulasimTuru = tur;
    o->veriler[i].mesafe = mesafe;
    o->veriler[i].haftalikGun = gun;
    o->veriler[i].yolcuSayisi = yolcu;
    o->veriler[i].aylikMesafe = aylikMesafe;
    o->veriler[i].karbonSalimi = karbon;

    printf("\nKayit basarili!\n");
    printf("Aylik mesafe: %.2f km\n", aylikMesafe);
    printf("Karbon salimi: %.3f kg CO2e\n", karbon);

    dosyayaKaydet();
}

/* 5. AYLIK KARBON RAPORU */
void aylikRapor()
{
    int i;
    float toplam = 0;

    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    printf("\n--- AYLIK KARBON RAPORU (%s) ---\n", o->ad);
    if (o->kayitSayisi == 0) { printf("Henuz veri girilmemis.\n"); return; }

    for (i = 0; i < o->kayitSayisi; i++) {
        toplam += o->veriler[i].karbonSalimi;
        printf("\nAy: %d\n", o->veriler[i].ay);
        printf("Aylik mesafe: %.2f km\n", o->veriler[i].aylikMesafe);
        printf("Karbon salimi: %.3f kg CO2e\n", o->veriler[i].karbonSalimi);
    }
    printf("\nToplam kayitli karbon salimi: %.3f kg CO2e\n", toplam);
}

/* 6. HEDEF BELÝRLEME */
void hedefBelirle()
{
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    printf("\n--- HEDEF BELIRLEME ---\n");
    printf("Azaltim hedefi yuzdesini giriniz (Orn: 15): ");
    scanf("%f", &o->hedefYuzde);

    if (o->hedefYuzde < 0 || o->hedefYuzde > 100) {
        printf("Hedef 0-100 arasinda olmalidir!\n");
        o->hedefYuzde = 15.0;
        return;
    }
    printf("Yeni hedef basariyla kaydedildi: %.2f%% azalis hedefi\n", o->hedefYuzde);
    dosyayaKaydet();
}

/* 7. GELÝÞMÝÞ HEDEF TAKÝP SÝSTEMÝ */
void hedefKontrol()
{
    int i, onceki = -1, simdiki = -1;
    float gerceklesenYuzde, hedefKarbonMiktari, farkKg;

    if (aktifOgrenciIndex == -1) { 
        printf("\nOnce giris yapin!\n"); 
        return; 
    }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    printf("\n====================================\n");
    printf("         HEDEF TAKIP ANALIZI\n");
    printf("====================================\n");

    if (o->kayitSayisi < 2) { 
        printf("Karsilastirma yapabilmek icin en az iki ay verisi gereklidir.\n"); 
        return; 
    }

    for (i = 0; i < o->kayitSayisi; i++) {
        if (simdiki == -1 || o->veriler[i].ay > o->veriler[simdiki].ay) {
            simdiki = i;
        }
    }

    for (i = 0; i < o->kayitSayisi; i++) {
        if (o->veriler[i].ay < o->veriler[simdiki].ay) {
            if (onceki == -1 || o->veriler[i].ay > o->veriler[onceki].ay) {
                onceki = i;
            }
        }
    }

    if (onceki == -1 || o->veriler[onceki].karbonSalimi <= 0) { 
        printf("Karsilastirilacak uygun onceki ay verisi bulunamadi.\n"); 
        return; 
    }

    float oncekiSalim = o->veriler[onceki].karbonSalimi;
    float simdikiSalim = o->veriler[simdiki].karbonSalimi;

    gerceklesenYuzde = ((oncekiSalim - simdikiSalim) / oncekiSalim) * 100;

    printf("Karsilastirilan Aylar : Ay %d (Onceki) -> Ay %d (Son)\n", o->veriler[onceki].ay, o->veriler[simdiki].ay);
    printf("Onceki Ay Salimi     : %.3f kg CO2e\n", oncekiSalim);
    printf("Son Ay Salimi        : %.3f kg CO2e\n", simdikiSalim);
    printf("Belirlenen Hedef     : %.1f%% azalis\n", o->hedefYuzde);
    printf("------------------------------------\n");

    if (gerceklesenYuzde >= 0) {
        printf("Gerceklesen Degisim  : %.2f%% azalis (Tasarruf)\n", gerceklesenYuzde);
        
        if (gerceklesenYuzde >= o->hedefYuzde) {
            printf("\n[TEBRIKLER!] Karbon azaltim hedefinize basariyla ulastiniz!\n");
        } else {
            hedefKarbonMiktari = oncekiSalim * (1.0 - (o->hedefYuzde / 100.0));
            farkKg = simdikiSalim - hedefKarbonMiktari;
            printf("\n[BILGI] Hedefinize henuz ulasamadiniz.\n");
            printf("Hedefi tutturmak icin son ay saliminizi **%.3f kg CO2e** daha dusurmeniz gerekiyordu.\n", farkKg);
        }
    } else {
        float artisYuzdesi = -gerceklesenYuzde;
        hedefKarbonMiktari = oncekiSalim * (1.0 - (o->hedefYuzde / 100.0));
        farkKg = simdikiSalim - hedefKarbonMiktari;
        
        printf("Gerceklesen Degisim  : %+.2f%% artis (Karbon yukseldi!)\n", artisYuzdesi);
        printf("\n[UYARI] Karbon saliminiz onceki aya gore artmis!\n");
        printf("Belirlenen hedefe ulasmak icin toplam **%.3f kg CO2e** azaltim yapmaniz gerekiyor.\n", farkKg);
    }
}

/* 8. GECMIS AYLARI KARSILASTIR */
void aylariKarsilastir()
{
    int i, j;
    KarbonVerisi gecici;
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    if (o->kayitSayisi == 0) { printf("Veri yok.\n"); return; }

    for (i = 0; i < o->kayitSayisi - 1; i++) {
        for (j = 0; j < o->kayitSayisi - i - 1; j++) {
            if (o->veriler[j].ay > o->veriler[j + 1].ay) {
                gecici = o->veriler[j];
                o->veriler[j] = o->veriler[j + 1];
                o->veriler[j + 1] = gecici;
            }
        }
    }

    printf("\n--- GECMIS AY KARSILASTIRMASI ---\n");
    for (i = 0; i < o->kayitSayisi; i++) {
        printf("Ay %d : %.3f kg CO2e\n", o->veriler[i].ay, o->veriler[i].karbonSalimi);
        if (i > 0) {
            if (o->veriler[i].karbonSalimi < o->veriler[i - 1].karbonSalimi) printf("  Salim azaldi.\n");
            else if (o->veriler[i].karbonSalimi > o->veriler[i - 1].karbonSalimi) printf("  Salim artti.\n");
            else printf("  Degisim yok.\n");
        }
    }
}

/* 9. SURDURULEBILIRLIK ONERILERI */
void oneriler()
{
    int i, enYuksek = -1;
    float maksimum = -1;
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    if (o->kayitSayisi == 0) { printf("Once ulasim bilgisi giriniz.\n"); return; }

    for (i = 0; i < o->kayitSayisi; i++) {
        if (o->veriler[i].karbonSalimi > maksimum) {
            maksimum = o->veriler[i].karbonSalimi;
            enYuksek = i;
        }
    }

    printf("\n--- SURDURULEBILIRLIK ONERILERI ---\n");
    printf("En yuksek karbon salimi: %.3f kg CO2e\n", maksimum);
    if (enYuksek != -1) {
        switch (o->veriler[enYuksek].ulasimTuru) {
            case 1: printf("Otobusu surdur, yuruyus veya bisiklet dene.\n"); break;
            case 2: printf("Toplu tasimaya devam et.\n"); break;
            case 3: printf("Ozel araci azalt, ortak arac veya toplu tasima kullan.\n"); break;
            case 4: case 5: printf("Harika, dusuk karbonlu ulasim!\n"); break;
        }
    }
}

/* 10. KAYITLI VERILERI GOSTER */
void tumKayitlariGoster()
{
    int i;
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    printf("\n--- KAYITLI VERILER (%s) ---\n", o->ad);
    for (i = 0; i < o->kayitSayisi; i++) {
        printf("Ay %d - %.3f kg CO2e\n", o->veriler[i].ay, o->veriler[i].karbonSalimi);
    }
    if (o->kayitSayisi == 0) printf("Kayit bulunamadi.\n");
}

/* 11. AGAC KARSILIGI HESAPLAMA */
void agacKarsiligiHesapla()
{
    int i;
    float enYuksek, enDusuk, fark, agacSayisi;
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    if (o->kayitSayisi == 0) { printf("Veri yok.\n"); return; }

    enYuksek = o->veriler[0].karbonSalimi;
    enDusuk = o->veriler[0].karbonSalimi;

    for (i = 1; i < o->kayitSayisi; i++) {
        if (o->veriler[i].karbonSalimi > enYuksek) enYuksek = o->veriler[i].karbonSalimi;
        if (o->veriler[i].karbonSalimi < enDusuk) enDusuk = o->veriler[i].karbonSalimi;
    }

    fark = enYuksek - enDusuk;
    agacSayisi = fark / AGAC_YILLIK_CO2;

    printf("\n--- AGAC KARSILIGI HESAPLAMA ---\n");
    printf("Aylik Salim Farki: %.3f kg CO2e\n", fark);
    printf("Bu farki dengelemek icin gereken yillik agac esdegeri: %.2f agac\n", agacSayisi);
}

/* 12. KAMPUS KARBON HARITASI (Fakülte Otomatik Toplam - En Düþük & En Yüksek) */
void kampusKarbonHaritasi()
{
    int i, j, k;
    char fakulteAdlari[MAX_OGRENCI][60];
    float fakulteToplamlari[MAX_OGRENCI];
    int fakulteSayisi = 0;
    int minIdx = 0, maxIdx = 0;

    if (ogrenciSayisi == 0) {
        printf("\nSistemde hic ogrenci kaydi yok!\n");
        return;
    }

    for (i = 0; i < ogrenciSayisi; i++) {
        float ogrenciToplamKarbon = 0;
        for (j = 0; j < ogrenciler[i].kayitSayisi; j++) {
            ogrenciToplamKarbon += ogrenciler[i].veriler[j].karbonSalimi;
        }

        int bulunanIndex = -1;
        for (k = 0; k < fakulteSayisi; k++) {
            if (strcmp(fakulteAdlari[k], ogrenciler[i].fakulte) == 0) {
                bulunanIndex = k;
                break;
            }
        }

        if (bulunanIndex != -1) {
            fakulteToplamlari[bulunanIndex] += ogrenciToplamKarbon;
        } else {
            strcpy(fakulteAdlari[fakulteSayisi], ogrenciler[i].fakulte);
            fakulteToplamlari[fakulteSayisi] = ogrenciToplamKarbon;
            fakulteSayisi++;
        }
    }

    for (i = 0; i < fakulteSayisi; i++) {
        if (fakulteToplamlari[i] > fakulteToplamlari[maxIdx]) {
            maxIdx = i;
        }
        if (fakulteToplamlari[i] < fakulteToplamlari[minIdx]) {
            minIdx = i;
        }
    }

    printf("\n--- KAMPUS KARBON HARITASI (OTOMATIK FAKULTE TOPLAMLARI) ---\n");
    for (i = 0; i < fakulteSayisi; i++) {
        printf("%-20s | ", fakulteAdlari[i]);
        int yildizSayisi = (int)(fakulteToplamlari[i] / 10.0);
        if (yildizSayisi > 30) yildizSayisi = 30;
        for (j = 0; j < yildizSayisi; j++) {
            printf("*");
        }
        printf(" (%.2f kg CO2e)\n", fakulteToplamlari[i]);
    }

    printf("\nKampusun En Cevreci Fakultesi (En Dusuk Salim): ?? **%s** (%.2f kg CO2e)\n", 
           fakulteAdlari[minIdx], fakulteToplamlari[minIdx]);
    printf("En yuksek karbon salimina sahip fakulte: **%s** (%.2f kg CO2e)\n", 
           fakulteAdlari[maxIdx], fakulteToplamlari[maxIdx]);
}

/* 13. GUNUN YESIL GOREVI */
void gununYesilGorevi()
{
    int secim;
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    printf("\n--- GUNUN YESIL GOREVI ---\n");
    printf("Aktif Ogrenci: %s | Mevcut Yesil Puaniniz: %d\n", o->ad, o->yesilPuan);
    printf("Gunun Gorevi: Bugun kampuse gelirken toplu tasima kullan veya en az 2 km yuru!\n");
    printf("Gorevi tamamladiniz mi?\n1- Evet (+10 Puan)\n2- Hayir\nSeciminiz: ");
    scanf("%d", &secim);

    if (secim == 1) {
        o->yesilPuan += 10;
        dosyayaKaydet();
        printf("\nTebrikler! 10 yesil puan kazandiniz. Toplam Puan: %d\n", o->yesilPuan);
    } else {
        printf("\nSorun deðil, yarin tekrar deneyebilirsin!\n");
    }
}

/* 14. SANAL KAMPÜS ORMANI SÝMÜLATÖRÜ */
void sanalKampusOrmani()
{
    int i;
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    int fidanSayisi = o->yesilPuan / 10;

    printf("\n====================================\n");
    printf("      SANAL KAMPUS ORMANI (%s)\n", o->ad);
    printf("====================================\n");
    printf("Yesil Puaniniz: %d Puan\n", o->yesilPuan);
    printf("Ormanindaki Toplam Agac Sayisi: %d\n", fidanSayisi);
    printf("------------------------------------\n");

    if (fidanSayisi == 0) {
        printf("?? Ormanin henuz bos! Gunun yesil gorevlerini tamamlayarak \n");
        printf("   veya dusuk karbonlu ulasim secerek fidan dikmeye basla.\n");
    } else {
        printf("Kampus Iklim Ormanin:\n");
        for (i = 0; i < fidanSayisi && i < 40; i++) {
            if (i % 5 == 0) printf("\n");
            if (i % 2 == 0) printf(" ?? ");
            else printf(" ?? ");
        }
        printf("\n\nHarika! Bu sanal agaclar dogaya olan katkini simgeliyor.\n");
    }
}

/* 15. AKILLI ROZET VE UNVAN SÝSTEMÝ */
void rozetVeUnvanlariGoster()
{
    int i;
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    int bisikletYuruyusVar = 0;
    int ozelAracTekbasina = 0;

    for (i = 0; i < o->kayitSayisi; i++) {
        if (o->veriler[i].ulasimTuru == 4 || o->veriler[i].ulasimTuru == 5) {
            bisikletYuruyusVar = 1;
        }
        if (o->veriler[i].ulasimTuru == 3 && o->veriler[i].yolcuSayisi == 1) {
            ozelAracTekbasina++;
        }
    }

    printf("\n====================================\n");
    printf("     AKILLI UNVAN VE ROZET SISTEMI\n");
    printf("====================================\n");
    printf("Ogrenci: %s | Bolum: %s\n", o->ad, o->fakulte);
    printf("------------------------------------\n");

    printf("Mevcut Unvanin: ");
    if (o->yesilPuan >= 40 && bisikletYuruyusVar) {
        printf("?? **Bas Kampus Cevrecisi (Elci)**\n");
    } else if (bisikletYuruyusVar) {
        printf("?? **Pedal / Yuruyus Dostu**\n");
    } else if (ozelAracTekbasina >= 2) {
        printf("?? **Asfalt Canavari** (Dikkat: Tek basina ozel arac salimi yuksek!)\n");
    } else {
        printf("?? **Yesil Aday**\n");
    }

    printf("\nRozet Durumlari:\n");
    if (o->yesilPuan >= 10) printf(" [x] ?? Ilk Adim Rozeti (Yesil gorev baslangici)\n");
    else printf(" [ ] ?? Ilk Adim Rozeti (Kilitli - 10 Yesil Puan gerek)\n");

    if (bisikletYuruyusVar) printf(" [x] ?? Temiz Ulasim Rozeti (Bisiklet veya yuruyus tercih edildi)\n");
    else printf(" [ ] ?? Temiz Ulasim Rozeti (Kilitli - Yuruyus/Bisiklet secilmeli)\n");

    if (o->kayitSayisi >= 2) printf(" [x] ?? Istikrar Rozeti (En az 2 ay veri girildi)\n");
    else printf(" [ ] ?? Istikrar Rozeti (Kilitli - 2 ay kayit gerek)\n");

    if (o->yesilPuan >= 40) printf(" [x] ?? Orman Muhafizi (40+ Yesil Puan toplandi)\n");
    else printf(" [ ] ?? Orman Muhafizi (Kilitli - 40 Puan gerek)\n");
}

/* 16. KAMPÜS YEÞÝL BÜTÇE VE PAZAR YERÝ SÝMÜLASYONU */
void yesilPazarYeri()
{
    if (aktifOgrenciIndex == -1) { printf("\nOnce giris yapin!\n"); return; }
    Ogrenci *o = &ogrenciler[aktifOgrenciIndex];

    printf("\n====================================\n");
    printf("    YESIL KAMPUS PAZAR YERI & ODULLER\n");
    printf("====================================\n");
    printf("Aktif Ogrenci: %s | Mevcut Yesil Puan: %d Puan\n", o->ad, o->yesilPuan);
    printf("------------------------------------\n");
    printf("Kullanilabilir Oduller ve Indirimler:\n");
    
    if (o->yesilPuan >= 20)
        printf(" [?] 1. Kampus Yemekhanesinde %%10 Indirim Hakki\n");
    else
        printf(" [ ] 1. Kampus Yemekhanesinde %%10 Indirim (Kilitli - 20 Puan gerek)\n");

    if (o->yesilPuan >= 35)
        printf(" [?] 2. Kutuphaneden Ucretsiz Fotokopi Hakki (50 Sayfa)\n");
    else
        printf(" [ ] 2. Kutuphaneden Ucretsiz Fotokopi (Kilitli - 35 Puan gerek)\n");

    if (o->yesilPuan >= 50)
        printf(" [?] 3. Kampus Senliklerine Oncelikli Bilet\n");
    else
        printf(" [ ] 3. Kampus Senlik Bileti (Kilitli - 50 Puan gerek)\n");
        
    if (o->yesilPuan < 20)
        printf("\n=> Daha fazla indirim acmak icin gunun yesil gorevlerini tamamla!\n");
}

/* DOSYAYA KAYDET */
void dosyayaKaydet()
{
    FILE *dosya = fopen("karbon_verileri.txt", "w");
    int i, j;
    if (dosya == NULL) return;

    fprintf(dosya, "%d\n", ogrenciSayisi);
    for (i = 0; i < ogrenciSayisi; i++) {
        fprintf(dosya, "%s %d %s %.2f %d %d\n", 
                ogrenciler[i].ad, 
                ogrenciler[i].ogrenciNo, 
                ogrenciler[i].fakulte, 
                ogrenciler[i].hedefYuzde, 
                ogrenciler[i].yesilPuan, 
                ogrenciler[i].kayitSayisi);

        for (j = 0; j < ogrenciler[i].kayitSayisi; j++) {
            fprintf(dosya, "%d %d %.2f %d %d %.2f %.3f\n",
                    ogrenciler[i].veriler[j].ay,
                    ogrenciler[i].veriler[j].ulasimTuru,
                    ogrenciler[i].veriler[j].mesafe,
                    ogrenciler[i].veriler[j].haftalikGun,
                    ogrenciler[i].veriler[j].yolcuSayisi,
                    ogrenciler[i].veriler[j].aylikMesafe,
                    ogrenciler[i].veriler[j].karbonSalimi);
        }
    }
    fclose(dosya);
}

/* DOSYADAN OKU */
void dosyadanOku()
{
    FILE *dosya = fopen("karbon_verileri.txt", "r");
    int i, j;
    if (dosya == NULL) return;

    if (fscanf(dosya, "%d", &ogrenciSayisi) != 1) {
        ogrenciSayisi = 0;
        fclose(dosya);
        return;
    }

    for (i = 0; i < ogrenciSayisi; i++) {
        fscanf(dosya, "%49s %d %59s %f %d %d", 
               ogrenciler[i].ad, 
               &ogrenciler[i].ogrenciNo, 
               ogrenciler[i].fakulte, 
               &ogrenciler[i].hedefYuzde, 
               &ogrenciler[i].yesilPuan, 
               &ogrenciler[i].kayitSayisi);

        for (j = 0; j < ogrenciler[i].kayitSayisi; j++) {
            fscanf(dosya, "%d %d %f %d %d %f %f",
                   &ogrenciler[i].veriler[j].ay,
                   &ogrenciler[i].veriler[j].ulasimTuru,
                   &ogrenciler[i].veriler[j].mesafe,
                   &ogrenciler[i].veriler[j].haftalikGun,
                   &ogrenciler[i].veriler[j].yolcuSayisi,
                   &ogrenciler[i].veriler[j].aylikMesafe,
                   &ogrenciler[i].veriler[j].karbonSalimi);
        }
    }
    fclose(dosya);

    if (ogrenciSayisi > 0) {
        aktifOgrenciIndex = 0;
    }
}

/* ANA MENU */
void menuGoster()
{
    printf("\n====================================\n");
    printf("            KARBONROTAM (KTU)\n");
    printf(" UNIVERSITE KARBON TAKIP SISTEMI\n");
    printf("====================================\n");
    if (aktifOgrenciIndex != -1) {
        printf(" Aktif Ogrenci: %s (%s)\n", ogrenciler[aktifOgrenciIndex].ad, ogrenciler[aktifOgrenciIndex].fakulte);
        printf("------------------------------------\n");
    } else {
        printf(" [!] Aktif ogrenci secilmedi!\n");
        printf("------------------------------------\n");
    }
    printf("1. Yeni Ogrenci Kaydi Ekle\n");
    printf("2. Ogrenci Degistir / Sec (Giris Yap)\n");
    printf("3. Aktif Ogrenci Bilgileri\n");
    printf("4. Ulasim Bilgisi Gir (Aktif Ogrenci)\n");
    printf("5. Aylik Karbon Raporu\n");
    printf("6. Karbon Azaltim Hedefi Belirle\n");
    printf("7. Hedef Takibi\n");
    printf("8. Gecmis Ay Karsilastirmasi\n");
    printf("9. Surdurulebilirlik Onerileri\n");
    printf("10. Kayitli Verileri Goster\n");
    printf("11. Agac Karsiligi Hesaplama\n");
    printf("12. Kampus Karbon Haritasi\n");
    printf("13. Gunun Yesil Gorevi\n");
    printf("14. Sanal Kampus Ormani\n");
    printf("15. Akilli Unvan ve Rozetler\n");
    printf("16. Kampus Yesil Pazar Yeri & Indirimler\n");
    printf("0. Cikis\n");
    printf("====================================\n");
    printf("Seciminiz: ");
}

/* ANA PROGRAM */
int main()
{
    int secim;
    dosyadanOku();

    do {
        menuGoster();
        if (scanf("%d", &secim) != 1) {
            printf("Lutfen sayisal bir secim giriniz.\n");
            while (getchar() != '\n');
            secim = -1;
            continue;
        }

        switch (secim) {
            case 1: ogrenciKayit(); break;
            case 2: ogrenciSecGiris(); break;
            case 3: ogrenciBilgisi(); break;
            case 4: ulasimBilgisiAl(); break;
            case 5: aylikRapor(); break;
            case 6: hedefBelirle(); break;
            case 7: hedefKontrol(); break;
            case 8: aylariKarsilastir(); break;
            case 9: oneriler(); break;
            case 10: tumKayitlariGoster(); break;
            case 11: agacKarsiligiHesapla(); break;
            case 12: kampusKarbonHaritasi(); break;
            case 13: gununYesilGorevi(); break;
            case 14: sanalKampusOrmani(); break;
            case 15: rozetVeUnvanlariGoster(); break;
            case 16: yesilPazarYeri(); break;
            case 0:
                dosyayaKaydet();
                printf("\nKarbonRotam kapatiliyor. Tesekkurler!\n");
                break;
            default:
                printf("Hatali secim! Tekrar deneyiniz.\n");
        }
    } while (secim != 0);

    return 0;
}
