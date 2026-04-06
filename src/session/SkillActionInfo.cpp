// Ref CSession::GetSkillActionInfo — decompiled from Ref/Session2.cpp (0x67a240).
#include "SkillActionInfo.h"

void GetSkillActionInfo(int skillId, int& beginEffectIdRef, int& motionTypeRef)
{
    int* beginEffectId = &beginEffectIdRef;
    int* motionType = &motionTypeRef;

    *beginEffectId = 16;
    *motionType = 7;
    if (skillId <= 384) {
        if (skillId == 384) {
$L153942:
            *motionType = 0;
            *beginEffectId = -1;
        } else {
            switch (skillId) {
            case 5:
            case 7:
            case 42:
            case 46:
            case 366:
                goto $L153913;
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 30:
            case 31:
            case 32:
            case 33:
            case 35:
            case 54:
            case 65:
            case 66:
            case 67:
            case 68:
            case 69:
            case 70:
            case 71:
            case 72:
            case 73:
            case 74:
            case 75:
            case 76:
            case 77:
            case 78:
            case 79:
            case 80:
            case 81:
            case 82:
            case 83:
            case 84:
            case 85:
            case 86:
            case 87:
            case 88:
            case 89:
            case 90:
            case 91:
            case 92:
            case 340:
            case 341:
            case 342:
                goto $L153985;
            case 28:
            case 34:
            case 136:
            case 156:
            case 157:
            case 334:
            case 335:
                goto $L153963;
            case 29:
            case 151:
            case 304:
                goto $L153907;
            case 45:
                *beginEffectId = 43;
                break;
            case 47:
            case 212:
            case 214:
            case 219:
            case 253:
            case 266:
            case 267:
            case 338:
            case 367:
            case 368:
            case 382:
                goto $L153937;
            case 51:
            case 55:
            case 60:
            case 63:
            case 64:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97:
            case 98:
            case 99:
            case 100:
            case 101:
            case 102:
            case 103:
            case 104:
            case 105:
            case 106:
            case 107:
            case 108:
            case 110:
            case 111:
            case 112:
            case 113:
            case 114:
            case 126:
            case 127:
            case 128:
            case 129:
            case 130:
            case 131:
            case 132:
            case 133:
            case 134:
            case 135:
            case 137:
            case 138:
            case 139:
            case 140:
            case 141:
            case 153:
            case 158:
            case 159:
            case 160:
            case 161:
            case 162:
            case 163:
            case 164:
            case 165:
            case 166:
            case 167:
            case 168:
            case 169:
            case 170:
            case 171:
            case 172:
            case 173:
            case 174:
            case 175:
            case 176:
            case 177:
            case 178:
            case 179:
            case 180:
            case 181:
            case 182:
            case 183:
            case 184:
            case 185:
            case 186:
            case 187:
            case 188:
            case 189:
            case 190:
            case 191:
            case 192:
            case 234:
            case 235:
            case 236:
            case 237:
            case 258:
            case 264:
            case 271:
            case 272:
            case 273:
            case 371:
            case 379:
            case 383:
                goto $L153924;
            case 56:
            case 58:
            case 61:
            case 62:
            case 109:
            case 263:
            case 370:
            case 372:
                goto $L153990;
            case 57:
                *beginEffectId = 144;
                *motionType = 2;
                break;
            case 59:
            case 149:
            case 152:
                goto $L153920;
            case 115:
            case 116:
            case 117:
            case 118:
            case 119:
            case 120:
            case 121:
            case 122:
            case 123:
            case 124:
            case 125:
                goto $L153928;
            case 196:
            case 209:
            case 249:
            case 252:
            case 257:
            case 268:
            case 349:
            case 350:
            case 356:
            case 380:
                goto $L153942;
            case 229:
            case 230:
            case 231:
            case 232:
            case 250:
            case 251:
                goto $L153946;
            case 243:
            case 244:
            case 247:
            case 337:
            case 344:
            case 345:
            case 346:
            case 365:
            case 373:
            case 374:
            case 375:
            case 381:
                goto $L153939;
            case 256:
            case 261:
            case 269:
            case 369:
                goto $L153970;
            case 306:
            case 307:
            case 308:
            case 309:
            case 310:
            case 311:
            case 312:
            case 313:
            case 325:
            case 327:
            case 328:
            case 329:
            case 330:
                goto $L153960;
            case 316:
            case 324:
                goto $L153987;
            case 317:
            case 319:
            case 320:
            case 321:
            case 322:
                *beginEffectId = -1;
                *motionType = 17;
                break;
            case 347:
                *beginEffectId = 454;
                break;
            case 357:
            case 359:
            case 360:
            case 361:
            case 362:
                goto $L153993;
            case 364:
                *motionType = 1;
                *beginEffectId = -1;
                break;
            default:
                return;
            }
        }
        return;
    }
    if (skillId <= 494) {
        if (skillId == 494) {
$L154005:
            *motionType = 23;
            *beginEffectId = -1;
        } else {
            switch (skillId) {
            case 387:
            case 411:
            case 444:
                goto $L153942;
            case 389:
            case 402:
            case 403:
            case 425:
            case 427:
            case 434:
            case 462:
            case 463:
            case 464:
            case 465:
            case 467:
            case 468:
            case 469:
            case 470:
            case 471:
            case 472:
            case 485:
            case 493:
                goto $L153939;
            case 394:
                goto $L153987;
            case 395:
            case 396:
            case 488:
$L153960:
                *beginEffectId = -1;
                *motionType = 16;
                break;
            case 397:
            case 398:
            case 399:
            case 484:
                goto $L153990;
            case 400:
            case 483:
                goto $L153985;
            case 401:
                goto $L153970;
            case 405:
            case 482:
                goto $L153993;
            case 406:
            case 459:
            case 479:
            case 486:
                goto $L153924;
            case 408:
            case 487:
            case 489:
                goto $L153963;
            case 412:
                *motionType = 21;
                *beginEffectId = -1;
                break;
            case 413:
                *motionType = 2;
                *beginEffectId = 435;
                break;
            case 414:
                *motionType = 22;
                *beginEffectId = -1;
                break;
            case 415:
                *motionType = 31;
                *beginEffectId = -1;
                break;
            case 416:
            case 445:
            case 447:
            case 448:
            case 449:
            case 450:
            case 451:
            case 452:
            case 453:
            case 454:
            case 455:
            case 456:
            case 457:
            case 458:
            case 460:
            case 461:
                goto $L154005;
            case 417:
                *motionType = 32;
                *beginEffectId = -1;
                break;
            case 418:
                *motionType = 24;
                *beginEffectId = -1;
                break;
            case 419:
                *motionType = 26;
                *beginEffectId = -1;
                break;
            case 420:
                *motionType = 25;
                *beginEffectId = -1;
                break;
            case 421:
                *motionType = 27;
                *beginEffectId = -1;
                break;
            case 426:
                *motionType = 20;
                *beginEffectId = -1;
                break;
            case 428:
            case 429:
            case 430:
                *motionType = 34;
                *beginEffectId = -1;
                break;
            case 431:
            case 432:
            case 433:
                *motionType = 33;
                *beginEffectId = -1;
                break;
            case 446:
            case 478:
            case 490:
                *motionType = -1;
                *beginEffectId = -1;
                break;
            case 480:
$L153946:
                *motionType = 12;
                *beginEffectId = -1;
                break;
            default:
                return;
            }
        }
        return;
    }
    if (skillId <= 668) {
        if (skillId < 666) {
            switch (skillId) {
            case 495:
            case 544:
            case 654:
            case 661:
            case 663:
                goto $L153924;
            case 499:
                goto $L153913;
            case 500:
                *motionType = 39;
                *beginEffectId = -1;
                break;
            case 501:
            case 504:
            case 505:
            case 506:
            case 516:
            case 517:
                *motionType = 38;
                *beginEffectId = -1;
                break;
            case 502:
            case 503:
            case 507:
            case 512:
            case 513:
            case 514:
            case 515:
            case 518:
            case 519:
            case 520:
                goto $L153937;
            case 508:
            case 521:
                *motionType = 40;
                *beginEffectId = -1;
                break;
            case 523:
            case 524:
            case 525:
            case 526:
            case 531:
            case 534:
            case 539:
            case 540:
            case 541:
            case 542:
            case 543:
                *motionType = 36;
                *beginEffectId = -1;
                break;
            case 527:
            case 528:
            case 529:
            case 532:
            case 535:
            case 536:
            case 537:
            case 538:
                *motionType = 35;
                *beginEffectId = -1;
                break;
            case 530:
$L153987:
                *motionType = 9;
                *beginEffectId = -1;
                break;
            case 572:
            case 573:
            case 574:
            case 575:
                goto $L154005;
            default:
                return;
            }
            return;
        }
        goto $L153924;
    }
    if (skillId <= 1006) {
        if (skillId == 1006)
            goto $L153985;
        if (skillId <= 690) {
            if (skillId == 690) {
$L153907:
                *beginEffectId = -1;
$L153970:
                *motionType = 0;
            } else {
                switch (skillId) {
                case 669:
                    goto $L153985;
                case 674:
                    goto $L153937;
                case 675:
                case 676:
                case 678:
                    goto $L153924;
                case 688:
$L153928:
                    *motionType = 5;
                    break;
                case 689:
$L153963:
                    *beginEffectId = -1;
                    goto $L153993;
                default:
                    return;
                }
            }
            return;
        }
        if (skillId > 1001) {
            if (skillId == 1004)
                goto $L153920;
            if (skillId != 1005)
                return;
        } else if (skillId != 1001) {
            if (skillId == 691) {
$L153993:
                *motionType = 7;
                return;
            }
            if (skillId != 694)
                return;
$L153985:
            *beginEffectId = 12;
            return;
        }
$L153937:
        *motionType = 2;
        *beginEffectId = -1;
        return;
    }
    if (skillId <= 8009) {
        if (skillId != 8009) {
            switch (skillId) {
            case 1009:
                goto $L153913;
            case 1011:
                *motionType = 16;
                *beginEffectId = -1;
                return;
            case 1013:
                *motionType = 5;
                *beginEffectId = -1;
                return;
            case 1015:
                goto $L153970;
            case 1016:
$L153990:
                *beginEffectId = -1;
$L153913:
                *motionType = 2;
                break;
            default:
                return;
            }
            return;
        }
$L153939:
        *motionType = 7;
$L153924:
        *beginEffectId = -1;
        return;
    }
    switch (skillId) {
    case 8012:
        goto $L153939;
    case 8204:
    case 8219:
    case 8220:
        goto $L153942;
    case 8206:
        goto $L153993;
    case 8222:
        goto $L153985;
    default:
        return;
    }

$L153920:
    *beginEffectId = -1;
    *motionType = 12;
}