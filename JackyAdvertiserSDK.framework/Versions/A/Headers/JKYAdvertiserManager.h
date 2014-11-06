//
// Created by Daniel Rinser on 26.05.14.
// Copyright (c) 2014 Jacky Media. All rights reserved.
//
// The software is provided "as-is", without warranty of any kind, express or implied,
// including but not limited to the warranties of merchantability, fitness for a particular
// purpose and non-infringement. In no event shall the authors or copyright holders be liable
// for any claim, damages or other liability, whether in an action of contract, tort or
// otherwise, arising from, out of or in connection with the software or the use or other
// dealings in the software.
//

#import <Foundation/Foundation.h>

extern NSString * const JKYAdvertiserSDKVersion;
extern NSString * const JKYAdvertiserErrorDomain;


@class SKProduct;
@class JKYAdvertiserManager;



/**
All errors returned by methods of this SDK are in the `JKYAdvertiserErrorDomain` error domain
and have one of the following error codes.

In most cases, the errors contain a human-readable description in `NSError`'s `localizedDescription`
(although that description is only provided in English - thus, you should not display those error
descriptions to your users - they are rather intended to give developers using this SDK a better
chance to understand what's wrong).
*/
typedef NS_ENUM(NSInteger, JKYAdvertiserErrorCode) {

    /** A generic network error. This can be caused by missing connectivity, timeout, or an error in the Jacky backend. */
    JKYAdvertiserErrorNetworkOperationFailed            = 0,
    
    /** Event tracking disabled, because this is no installation that comes through the Jacky network */
    JKYAdvertiserErrorNoAffiliateInstall                = 1,

    /** No app credentials (API key and API secret) specified */
    JKYAdvertiserErrorMissingAppCredentials             = 100,
    
    /** Invalid API key. Please double-check the key you provided to -startWithAPIKey:andAPISecret:. */
    JKYAdvertiserErrorInvalidAPIKey                     = 101,
    
    /** Invalid API secret. Please double-check the secret you provided to -startWithAPIKey:andAPISecret:. */
    JKYAdvertiserErrorInvalidAPISecret                  = 102,

    /** Advertising identifier not available. This is expected behavior on iOS version < 6.0. If running on iOS 6.0+, make sure the AdSupport framework is linked. */
    JKYAdvertiserErrorMissingAdvertisingIdentifier      = 200,
    
    /** One or more mandatory parameters are missing */
    JKYAdvertiserErrorMissingParameter                  = 201,

    /** The specified custom event name is not valid */
    JKYAdvertiserErrorInvalidCustomEventName            = 300,
    
    /** The specified user info for the custom event is not valid. */
    JKYAdvertiserErrorInvalidCustomEventUserInfo        = 301,
    
    /** The specified custom event does not exist. Please define custom events in your advertiser console prior to tracking them. */
    JKYAdvertiserErrorNoSuchCustomEvent                 = 302,
    
    /** The tracked event exists, but has not yet been verified. Please track this event once with test mode enabled, to verify it and allow tracking. */
    JKYAdvertiserErrorEventNotVerified                  = 303,
};


/**
You can adopt this delegate protocol to get informed about certain events like tracking errors.
*/
@protocol JKYAdvertiserManagerDelegate <NSObject>

@optional

/**
Called whenever an event tracking request failed for some reason.

If the error is a precondition error, this method is called synchronously withing the `track...` method.
If, however, the error is a networking or backend error, this method is called asynchronously and can happen
a significant amount of time after the tracking has been triggered (even after the next app launch, if the user
has been offline until then, for example).

Please note that *recoverable* issues (like temporary networking unavailability and timeouts) are not reported as
errors, because the requests will be automatically retried later.

@param  manager The shared `<JKYAdvertiserManager>` instance.
@param  error   The error that has occurred. This error is in the `JKYAdvertiserErrorDomain` error domain.
                Please see `<JKYAdvertiserErrorCode>` for a list of possible error types.
*/
- (void)jackyAdvertiserManager:(JKYAdvertiserManager *)manager trackingRequestDidFailWithError:(NSError *)error;


/**
Called after first launch, if Jacky has detected that the user was led to this app by a campaign that includes a deeplink URL.

This delegate method is called asynchronously and the exact moment at which it is called depends heavily on the network
connection conditions, because server communication has to take place to get this information. However, it will happen as
soon as possible after launch.

@param  manager        The shared `<JKYAdvertiserManager>` instance.
@param  deeplinkURL    The deeplink URL corresponding to the campaign that the user has clicked.
*/
- (void)jackyAdvertiserManager:(JKYAdvertiserManager *)manager didFindDeeplinkURL:(NSURL *)deeplinkURL;

@end


/**
This class is the main entry point into the SDK.

### Setup

To use the SDK to track any events, you first have to identify your app.

In your app delegate's `application:didFinishLaunchingWithOptions:`, call `<startWithAPIKey:andAPISecret:>` to setup the Jacky Advertiser SDK:

    [[JKYAdvertiserManager sharedManager] startWithAPIKey:@"<YOUR API KEY>"
                                             andAPISecret:@"<YOUR API SECRET>"];

That's all you have to do. You can find your API key and secret in the [Jacky Advertiser dashboard](https://www.jackymedia.com/advertiser/).


### Test mode

To make initial testing easier for you, we provide a **test mode**, which allows you to test all features of the SDK without further
configuration in the Jacky Advertiser Dashboard.

**Important: Before you can submit actual tracking requests, you need to hit every tracking event once with test mode enabled!
This is a safety net to ensure that your tracking code is correct. You can see the verification status for every event type
in your [Jacky Advertiser Dashboard](https://www.jackymedia.com/advertiser/).**

By default, test mode is disabled. You can toggle test mode via the `<testModeEnabled>` property. Make sure you do
this **before** calling `<startWithAPIKey:andAPISecret:>`:

    [JKYAdvertiserManager sharedManager].testModeEnabled = YES;

    [[JKYAdvertiserManager sharedManager] startWithAPIKey:@"<YOUR API KEY>"
                                             andAPISecret:@"<YOUR API SECRET>"];

**Don't forget to disable test mode in your production code!**


### Tracking Events

The Jacky Advertiser SDK allows you to track several different kinds of events, like user registrations, in-app purchases, or even custom events.

In general, the event tracking API is "fire and forget" - just submit your tracking request and forget about it. The Jacky SDK will do
the heavy lifting and ensure that all events are delivered by repeating failed requests, monitoring the device's connectivity and potentially
storing unsent tracking requests in a persistent queue for later retrying.

For this reason, the tracking API does not allow to pass completion blocks/callbacks. However, in case you are interested in failures (which
is generally a good idea, especially during development), you can register a `<delegate>`, see `<JKYAdvertiserManagerDelegate>`.

You can track:

* User registrations: `<trackRegistrationOfUserWithId:andName:>`
* In-app purchases: `<trackInAppPurchaseWithProduct:>` or `<trackInAppPurchaseWithProductIdentifier:price:currencyCode:>`
* Custom events: `<trackCustomEvent:userInfo:>`

*/
@interface JKYAdvertiserManager : NSObject

/**
================================================================================
@name   Shared Instance
================================================================================
*/

/**
Get the shared instance of the `JKYAdvertiserManager`.
Use this method to access this class from anywhere in your project.

@return The `JKYAdvertiserManager` shared instance.
*/
+ (JKYAdvertiserManager *)sharedManager;



/**
================================================================================
@name   Initialize
================================================================================
*/


/**
Initialize the Jacky Advertiser SDK for your app.

Typically, you want to do that in your app delegate's `application:didFinishLaunchingWithOptions:`.
You can obtain the required API key and secret in your [Jacky Advertiser dashboard](https://www.jackymedia.com/advertiser/) after creating an advertiser account.

@param  apiKey      Your API key.
@param  apiSecret   Your API secret.

@warning **Important:** You need to call this method prior to executing any actions (like tracking events).
*/
- (void)startWithAPIKey:(NSString *)apiKey
           andAPISecret:(NSString *)apiSecret;



/**
================================================================================
@name   Configure
================================================================================
*/

/**
Enable or disable test mode. This mode is intended for testing purposes to get you started quickly with Jacky.

Defaults to `NO`.

@warning **Please note:** Even with test mode enabled, you still need a Jacky advertiser account and call `<startWithAPIKey:andAPISecret:>` with your API key and API secret!
@warning **Important:** You need to call this method prior to executing `<startWithAPIKey:andAPISecret:>`!
*/
@property (nonatomic) BOOL testModeEnabled;

/**
Set a delegate to get informed about certain events like event tracking errors.
*/
@property (nonatomic, weak) id<JKYAdvertiserManagerDelegate> delegate;



/**
================================================================================
@name   Event tracking
================================================================================
*/

/**
Whenever a user has successfully signed up to your service, call this method to track the registration.

These events are used to correlate your registered users with their device identifiers in the Jacky Advertiser Dashboard.

@param  userId      An arbitrary string that can be used to uniquely identify a single user in your system (user ID, email, etc.)
@param  userName    An arbitrary user name (purely informational, will be used in the Jacky Advertiser Dashboard)
*/
- (void)trackRegistrationOfUserWithId:(NSString *)userId
                              andName:(NSString *)userName;


/**
Track a successful in-app purchase with Jacky, providing an `SKProduct` instance obtained from StoreKit.

In case you want to provide a product identifier, price and currency individually (for example, because
you don't have the `SKProduct` instance anymore at the time of the purchase completion), you can also use
`<trackInAppPurchaseWithProductIdentifier:price:currencyCode:>`.

@param  product The StoreKit product instance representing the purchased product.

@see    trackInAppPurchaseWithProductIdentifier:price:currencyCode:
 */
- (void)trackInAppPurchaseWithProduct:(SKProduct *)product;

/**
Track a successful in-app purchase with Jacky, providing a product identifier, price and currency.

In case you have an `SKProduct` instance at the time of the purchase completion, you should rather
use `<trackInAppPurchaseWithProduct:>` for your convenience.

@param  productIdentifier   The iTunes product identifier of the purchased product.
@param  price               The purchased product's price.
@param  currencyCode        The currency of `price` (as a 3-letter [ISO 4217 code](http://en.wikipedia.org/wiki/ISO_4217)).

@see trackInAppPurchaseWithProduct:
*/
- (void)trackInAppPurchaseWithProductIdentifier:(NSString *)productIdentifier
                                          price:(NSDecimalNumber *)price
                                   currencyCode:(NSString *)currencyCode;

/**
Track a custom event.

@warning **Please note:** Before you can track custom events, you need to define them in your [Jacky Advertiser dashboard](https://www.jackymedia.com/advertiser/).

@param  eventName   The name of the event as defined in your Jacky Advertiser dashboard.
@param  userInfo    Optional user info data. This can contain any structure that can be converted into valid JSON as defined by
                    [`NSJSONSerialization`](https://developer.apple.com/library/ios/documentation/foundation/reference/nsjsonserialization_class/Reference/Reference.html).
*/
- (void)trackCustomEvent:(NSString *)eventName
                userInfo:(NSDictionary *)userInfo;

@end
