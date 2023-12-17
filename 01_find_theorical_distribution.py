import numpy as np
import matplotlib.pyplot as plt
import yfinance as yf
from scipy.optimize import minimize
from distfit import distfit


# import quantstats as qs
# qs.reports.html(stock_ret, benchmark, output="{}.html".format(ticket.lower()))


def model_brownian_gbm(random_generator, dfit, paths, a, b, S, r, q, sigma, dt):
    Z = next(random_generator(dfit, paths, a, b))
    '''
    return S * np.exp(
        (r - q - np.power(sigma, np.array(2)) / np.array(2)) * dt +
        (Z * sigma) * np.sqrt(dt)
    )
    '''
    return np.exp(
        (r - q - sigma ** 2.0 / 2.0) * dt +
        (Z * sigma) * np.sqrt(dt)
    )


def generate_gbm(steps, random_generator, dfit, paths, a, b, S, r, q, sigma):
    dt = 1.0 / steps
    for _ in range(steps):
        yield model_brownian_gbm(random_generator, dfit, paths, a, b, S, r, q, sigma, dt)


def random_generator(dfit, paths, a, b):
    while True:
        yield np.random.normal(0, 1, paths)
        # yield (dfit.generate(n=paths) - np.abs(a)) / np.abs(b)


def minimize_mu_sigma(x, dfit, business_day, paths, returns_real, mu_objetive, sigma_objetive):
    """
    search a and b to minimize
    :return:
    """
    a, b, mu, sigma, q = x
    steps = len(returns_real)
    closed_estimated = list(generate_gbm(steps, random_generator, dfit, paths, a, b, 100, mu, 0.0, sigma))
    returns_estimated, mu_estimated, sigma_estimated = compute_stock(closed_estimated, business_day, period)
    return np.sum(np.power(mu_objetive - mu_estimated, 2))


def download_close(ticket):
    return yf.download(tickers=ticket, period='max', proxy=None)["Close"]


def compute_stock(close, business_day=252, period=1):
    returns = np.diff(np.log(close))
    mu = (np.mean(returns) / period) * business_day
    sigma = (returns.std() / np.sqrt(period)) * np.sqrt(business_day)
    return returns, mu, sigma


def optmize_gbm(ticket, period, business_day, paths, q, method='parametric', distr='popular', alpha=0.01):

    close = download_close(ticket)

    returns, mu, sigma = compute_stock(close, business_day, period)
    print('mu real = {}'.format(mu))
    print('sigma real = {}'.format(sigma))

    dfit = distfit(method, distr=distr, alpha=alpha)
    results = dfit.fit_transform(returns, verbose=0)

    # dfit.plot(chart='pdf')
    # dfit.plot(chart='cdf')
    # plt.show()

    loc = results['model']['loc']
    scale = results['model']['scale']
    results = minimize(minimize_mu_sigma, [loc, scale, mu, sigma, q], method='SLSQP',
                       args=(dfit, business_day, paths, returns, mu, sigma),
                       options={'max_iter': 100, 'disp': True})
    print(results)
    return dfit, mu, sigma, results.x


if __name__ == '__main__':

    ticket = 'TSLA'
    benchmark = 'SPY'
    steps = 500
    paths = 100
    r = 0.09
    q = 0.01
    business_day = 255
    period = 1  # daily=1, weekly=7

    dfit, mu, sigma, params = optmize_gbm(ticket, period, business_day, paths, q, method='parametric', distr='popular', alpha=0.01)
    print('mu real = {}'.format(mu))
    print('sigma real = {}'.format(sigma))
    '''
    mu real = 1.4642866726090733
    sigma real = 0.5708714226038419
    '''
    a, b, mu, sigma, q = params
    closed_optimized = list(generate_gbm(steps, random_generator, dfit,
                                         paths, a, b,
                                         100, mu, q, sigma))
    _, mu_optimized, sigma_optimized = compute_stock(closed_optimized, business_day, period)
    '''
    mu optimized = 0.999999959764252
    sigma optimized = 2.5446384610479505e-05
    '''
    print('a = {}'.format(a))
    print('b = {}'.format(b))
    print('q = {}'.format(q))
    print('mu = {}'.format(mu))
    print('sigma = {}'.format(sigma))
    print('mu measured = {}'.format(mu_optimized))
    print('sigma measured = {}'.format(sigma_optimized))

    plt.plot(list(range(len(closed_optimized))), closed_optimized)
    plt.show()
